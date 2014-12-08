/*
* 
* Copyright (c) 2014, Wieden+Kennedy, 
* Stephen Schieberl
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "BodyInterface.h"

#include "boost/algorithm/string.hpp"
#include "cinder/Utilities.h"
#include <vector>

using namespace ci;
using namespace std;

BodyInterface::BodyInterface()
: ProtocolInterface()
{
}

void BodyInterface::append( const ci::Buffer& buffer )
{
	size_t sz	= 0;
	size_t len	= buffer.getDataSize();
	if ( mBody ) {
		sz = mBody.getDataSize();
		mBody.resize( sz + len );
	} else {
		mBody = Buffer( len );
	}
	char_traits<char>::copy( (char*)mBody.getData() + sz, (char*)buffer.getData(), len );
}

const Buffer& BodyInterface::getBody() const
{
	return mBody;
}

void BodyInterface::setBody( const Buffer& body )
{
	size_t sz = body.getDataSize();
	if ( mBody && sz > 0 ) {
		mBody.resize( sz );
	} else {
		mBody = Buffer( sz );
	}
	char_traits<char>::copy( (char*)mBody.getData(), (char*)body.getData(), sz );
}

Buffer BodyInterface::toBuffer() const
{
	return mBody;
}

string BodyInterface::toString() const
{
	return bufferToString( mBody );
}
