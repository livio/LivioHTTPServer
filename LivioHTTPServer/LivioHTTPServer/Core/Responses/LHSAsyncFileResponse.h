/**
 * This library is covered under the BSD 2-clause license below.
 * Software License Agreement (BSD License)
 *
 * ------------------------------------------------------------
 *
 * Copyright (c) 2011, Deusty, LLC
 * All rights reserved.
 *
 * Redistribution and use of this software in source and binary forms,
 * with or without modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer.
 *
 * * Neither the name of Deusty nor the names of its
 *   contributors may be used to endorse or promote products
 *   derived from this software without specific prior
 *   written permission of Deusty, LLC.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ------------------------------------------------------------
 *
 * This library was forked from it's original version, which can be found here: https://github.com/robbiehanson/CocoaHTTPServer
 * This library retains it's original license and is maintained by Livio.
 */

#import <Foundation/Foundation.h>
#import "LHSResponse.h"

@class LHSConnection;

/**
 * This is an asynchronous version of HTTPFileResponse.
 * It reads data from the given file asynchronously via GCD.
 * 
 * It may be overriden to allow custom post-processing of the data that has been read from the file.
 * An example of this is the HTTPDynamicFileResponse class.
**/

@interface LHSAsyncFileResponse : NSObject <LHSResponse> {
    LHSConnection *connection;

    NSString *filePath;
    UInt64 fileLength;
    UInt64 fileOffset; // File offset as pertains to data given to connection
    UInt64 readOffset; // File offset as pertains to data read from file (but maybe not returned to connection)

    BOOL aborted;

    NSData *data;

    int fileFD;
    void *readBuffer;
    NSUInteger readBufferSize;   // Malloced size of readBuffer
    NSUInteger readBufferOffset; // Offset within readBuffer where the end of existing data is
    NSUInteger readRequestLength;
    dispatch_queue_t readQueue;
    dispatch_source_t readSource;
    BOOL readSourceSuspended;
}

- (id)initWithFilePath:(NSString *)filePath forConnection:(LHSConnection *)connection;
- (NSString *)filePath;

@end

/**
 * Explanation of Variables (excluding those that are obvious)
 * 
 * fileOffset
 *   This is the number of bytes that have been returned to the connection via the readDataOfLength method.
 *   If 1KB of data has been read from the file, but none of that data has yet been returned to the connection,
 *   then the fileOffset variable remains at zero.
 *   This variable is used in the calculation of the isDone method.
 *   Only after all data has been returned to the connection are we actually done.
 * 
 * readOffset
 *   Represents the offset of the file descriptor.
 *   In other words, the file position indidcator for our read stream.
 *   It might be easy to think of it as the total number of bytes that have been read from the file.
 *   However, this isn't entirely accurate, as the setOffset: method may have caused us to
 *   jump ahead in the file (lseek).
 * 
 * readBuffer
 *   Malloc'd buffer to hold data read from the file.
 * 
 * readBufferSize
 *   Total allocation size of malloc'd buffer.
 * 
 * readBufferOffset
 *   Represents the position in the readBuffer where we should store new bytes.
 * 
 * readRequestLength
 *   The total number of bytes that were requested from the connection.
 *   It's OK if we return a lesser number of bytes to the connection.
 *   It's NOT OK if we return a greater number of bytes to the connection.
 *   Doing so would disrupt proper support for range requests.
 *   If, however, the response is chunked then we don't need to worry about this.
 *   Chunked responses inheritly don't support range requests.
**/
