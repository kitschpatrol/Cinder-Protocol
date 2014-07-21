#include "cinder/app/AppBasic.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Text.h"

#include "boost/algorithm/string.hpp"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "TcpClient.h"

class HttpClientApp : public ci::app::AppBasic 
{
public:
	void						draw();
	void						setup();
	void						update();
private:
	TcpClientRef				mClient;
	TcpSessionRef				mSession;
	std::string					mHost;
	int32_t						mPort;
	
	HttpRequest					mHttpRequest;
	HttpResponse				mHttpResponse;
	
	void						write();
	
	void						onClose();
	void						onConnect( TcpSessionRef session );
	void						onError( std::string err, size_t bytesTransferred );
	void						onRead( ci::Buffer buffer );
	void						onReadComplete();
	void						onResolve();
	void						onWrite( size_t bytesTransferred );
	
	ci::Font					mFont;
	std::vector<std::string>	mText;
	ci::gl::TextureRef			mTexture;

	float						mFrameRate;
	bool						mFullScreen;
	ci::params::InterfaceGlRef	mParams;
};

#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

void HttpClientApp::draw()
{
	gl::clear( Colorf::black() );
	gl::setMatricesWindow( getWindowSize() );
	
	if ( mTexture ) {
		gl::draw( mTexture, Vec2i( 250, 20 ) );
	}
	
	mParams->draw();
}

void HttpClientApp::onClose()
{
	mText.push_back( "Disconnected" );
}

void HttpClientApp::onConnect( TcpSessionRef session )
{
	mHttpResponse	= HttpResponse();
	mSession		= session;
	mText.push_back( "Connected" );
	
	mSession->connectCloseEventHandler( &HttpClientApp::onClose, this );
	mSession->connectErrorEventHandler( &HttpClientApp::onError, this );
	mSession->connectReadCompleteEventHandler( &HttpClientApp::onReadComplete, this );
	mSession->connectReadEventHandler( &HttpClientApp::onRead, this );
	mSession->connectWriteEventHandler( &HttpClientApp::onWrite, this );
	
	mSession->write( mHttpRequest.toBuffer() );
}

void HttpClientApp::onError( string err, size_t bytesTransferred )
{
	string text = "Error";
	if ( !err.empty() ) {
		text += ": " + err;
	}
	mText.push_back( text );
}

void HttpClientApp::onRead( ci::Buffer buffer )
{
	mText.push_back(toString( buffer.getDataSize() ) + " bytes read" );
	
	if ( !mHttpResponse.hasHeader() ) {
		mHttpResponse.parseHeader( HttpResponse::bufferToString( buffer ) );
		buffer = HttpResponse::removeHeader( buffer );
	}
	mHttpResponse.append( buffer );
	mSession->read();
}

void HttpClientApp::onReadComplete()
{
	mText.push_back("Read complete" );
		
	console() << "HTTP version: ";
	switch ( mHttpResponse.getHttpVersion() ) {
		case HttpVersion::HTTP_0_9:
			console() << "0.9";
			break;
		case HttpVersion::HTTP_1_0:
			console() << "1.0";
			break;
		case HttpVersion::HTTP_1_1:
			console() << "1.1";
			break;
		case HttpVersion::HTTP_2_0:
			console() << "2.0";
			break;
	}
	console() << endl;
	console() << "Status code: " << mHttpResponse.getStatusCode() << endl;
	console() << "Reason: " << mHttpResponse.getReason() << endl;
	
	console() << "Headers: " << endl;
	for ( const KeyValuePair& kvp : mHttpResponse.getHeaders() ) {
		console() << ">> " << kvp.first << ": " << kvp.second << endl;
	}
	console() << endl;
	
	console() << "Response buffer:" << endl;
	console() << mHttpResponse << endl;
	
	mSession->close();
}

void HttpClientApp::onResolve()
{
	mText.push_back( "Endpoint resolved" );
}

void HttpClientApp::onWrite( size_t bytesTransferred )
{
	mText.push_back(toString( bytesTransferred ) + " bytes written" );
	mSession->read();
}

void HttpClientApp::setup()
{
	gl::enable( GL_TEXTURE_2D );
	
	mFont			= Font( "Georgia", 24 );
	mFrameRate		= 0.0f;
	mFullScreen		= false;
	mHost			= "libcinder.org";
	mPort			= 80;
	
	mHttpRequest = HttpRequest( "GET", "/", HttpVersion::HTTP_1_0 );
	mHttpRequest.setHeader( "Host", mHost );
	mHttpRequest.setHeader( "Accept", "*/*" );
	mHttpRequest.setHeader( "Connection", "close" );

	mParams = params::InterfaceGl::create( "Params", Vec2i( 200, 150 ) );
	mParams->addParam( "Frame rate",	&mFrameRate,					"", true );
	mParams->addParam( "Full screen",	&mFullScreen,					"key=f" );
	mParams->addParam( "Host",			&mHost );
	mParams->addParam( "Port",			&mPort,							"min=0 max=65535 step=1 keyDecr=p keyIncr=P" );
	mParams->addButton( "Write", bind(	&HttpClientApp::write, this ),	"key=w" );
	mParams->addButton( "Quit", bind(	&HttpClientApp::quit, this ),	"key=q" );
	
	mClient = TcpClient::create( io_service() );
	mClient->connectConnectEventHandler( &HttpClientApp::onConnect, this );
	mClient->connectErrorEventHandler( &HttpClientApp::onError, this );
	mClient->connectResolveEventHandler( &HttpClientApp::onResolve, this );
}

void HttpClientApp::update()
{
	mFrameRate = getFrameRate();
	
	if ( mFullScreen != isFullScreen() ) {
		setFullScreen( mFullScreen );
		mFullScreen = isFullScreen();
	}

	if ( !mText.empty() ) {
		TextBox tbox = TextBox().alignment( TextBox::LEFT ).font( mFont ).size( Vec2i( getWindowWidth() - 250, TextBox::GROW ) ).text( "" );
		for ( vector<string>::const_reverse_iterator iter = mText.rbegin(); iter != mText.rend(); ++iter ) {
			tbox.appendText( "> " + *iter + "\n" );
		}
		tbox.setColor( ColorAf( 1.0f, 0.8f, 0.75f, 1.0f ) );
		tbox.setBackgroundColor( ColorAf::black() );
		tbox.setPremultiplied( false );
		mTexture = gl::Texture::create( tbox.render() );
		while ( mText.size() > 75 ) {
			mText.erase( mText.begin() );
		}
	}
}

void HttpClientApp::write()
{
	// This sample is meant to work with only one session at a time
	if ( mSession && mSession->getSocket()->is_open() ) {
		return;
	}
		
	mText.push_back( "Connecting to:\n" + mHost + ":" + toString( mPort ) );
	
	mClient->connect( mHost, (uint16_t)mPort );		
}

CINDER_APP_BASIC( HttpClientApp, RendererGl )
