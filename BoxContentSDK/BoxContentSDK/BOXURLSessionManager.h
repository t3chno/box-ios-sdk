//
//  BOXURLSessionManager.h
//  BoxContentSDK
//
//  Created by Thuy Nguyen on 12/15/16.
//  Copyright © 2016 Box. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BOXURLSessionCacheClient.h"

NS_ASSUME_NONNULL_BEGIN

@protocol BOXURLSessionTaskDelegate <NSObject>

/**
 * To be called to finish the operation for a NSURLSessionTask upon its completion
 *
 * @param response      The response received from Box as a result of the API call.
 * @param responseData  The response data received from Box as a result of the API call. Will be nil for foreground session tasks
 * @param error         An error in the NSURLErrorDomain
 */
- (void)sessionTask:(NSURLSessionTask *)sessionTask didFinishWithResponse:(NSURLResponse *)response responseData:(NSData *)responseData error:(NSError *)error;

@optional

/**
 * To be called to process the intermediate response from the task
 *
 * @param response  The intermediate response received from Box as a result of the API call
 */
- (void)sessionTask:(NSURLSessionTask *)sessionTask processIntermediateResponse:(NSURLResponse *)response;

/**
 * To be called to process the intermediate data from the task
 *
 * @param data  The intermediate data received from Box as a result of the API call
 */
- (void)sessionTask:(NSURLSessionTask *)sessionTask processIntermediateData:(NSData *)data;

@end


@protocol BOXURLSessionDownloadTaskDelegate <BOXURLSessionTaskDelegate>

/**
 * Destination file path to move downloaded file into
 */
- (NSString *)destinationFilePath;

@optional

/**
 * Notify delegate about download progress
 */
- (void)downloadTask:(NSURLSessionDownloadTask *)downloadTask
   didWriteTotalBytes:(int64_t)totalBytesWritten
totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite;

@end


@protocol BOXURLSessionUploadTaskDelegate <BOXURLSessionTaskDelegate>

@optional

/**
 * Notify delegate about upload progress
 */
- (void)sessionTask:(NSURLSessionTask *)sessionTask
    didSendTotalBytes:(int64_t)totalBytesSent
totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend;

@end


@protocol BOXURLSessionManagerDelegate <BOXURLSessionCacheClientDelegate>
@end

/**
 This class is responsible for creating different NSURLSessionTask
 */
@interface BOXURLSessionManager : NSObject

/**
 * Should only use sharedInstance instead of creating a new instance of BOXURLSessionManager
 * BOXURLSessionManager is responsible for a unique background NSURLSession for the app
 * with BOXURLSessionManager itself as delegate
 */
+ (BOXURLSessionManager *)sharedInstance;

/**
 * This method needs to be called once in main app to set up the manager to be ready to
 * support background upload/download tasks.
 * If this method has not been called, all background task creations will fail
 *
 * @param delegate          used for encrypting/decrypting metadata cached for background session tasks
 * @param rootCacheDir      root directory for caching background session tasks' data
 */
- (void)oneTimeSetUpInAppToSupportBackgroundTasksWithDelegate:(id<BOXURLSessionManagerDelegate>)delegate rootCacheDir:(NSString *)rootCacheDir;

/**
 * This method needs to be called once in app extensions to set up the manager to be ready to
 * support background upload/download tasks.
 * If this method has not been called, all background task creations will fail
 *
 * @param backgroundSessionId background session id to create background session with
 * @param delegate          used for encrypting/decrypting metadata cached for background session tasks
 * @param rootCacheDir      root directory for caching background session tasks' data. Should be the same
 *                          as rootCacheDir for main app to allow main app takes over background session
 *                          tasks created from extensions
 */
- (void)oneTimeSetUpInExtensionToSupportBackgroundTasksWithBackgroundSessionId:(NSString *)backgroundSessionId delegate:(id<BOXURLSessionManagerDelegate>)delegate rootCacheDir:(NSString *)rootCacheDir;

/**
 * This method results in this BOXURLSessionManager becomes the delegate for session with backgroundSessionId identifier
 * should share the same rootCacheDir as the main app to work properly
 */
- (void)reconnectWithBackgroundSessionId:(NSString *)backgroundSessionId;

/**
 Create a NSURLSessionDataTask which does not need to be run in background,
 and its completionHandler will be called upon completion of the task
 */
- (NSURLSessionDataTask *)dataTaskWithRequest:(NSURLRequest *)request completionHandler:(void (^)(NSData * data, NSURLResponse * response, NSError * error))completionHandler;

/**
 Create a NSURLSessionDataTask which can be run in foreground to download data
 */
- (NSURLSessionDataTask *)foregroundDownloadTaskWithRequest:(NSURLRequest *)request taskDelegate:(id <BOXURLSessionDownloadTaskDelegate>)taskDelegate;

/**
 Create a foreground upload task given stream request
 */
- (NSURLSessionUploadTask *)foregroundUploadTaskWithStreamedRequest:(NSURLRequest *)request taskDelegate:(id <BOXURLSessionUploadTaskDelegate>)taskDelegate;

/**
 Retrieve a NSURLSessionDownloadTask to be run in the background to download file into a destination file path.
 If there is an existing task for userId and associateId, return that, else create a new one

 @param request         request to create download task with
 @param taskDelegate    the delegate to receive callback for the session task
 @param userId          userId that this task belongs to
 @param associateId     an id to associate with this session task to retrieve cache for or clean up later

 @return a background download task. Nil if already completed
 */
- (NSURLSessionDownloadTask *)backgroundDownloadTaskWithRequest:(NSURLRequest *)request taskDelegate:(id <BOXURLSessionDownloadTaskDelegate>)taskDelegate userId:(NSString *)userId associateId:(NSString *)associateId error:(NSError **)error;

/**
 Retrieve a NSURLSessionDownloadTask given a resume data to be run in the background to download file into a destination file path.

 @param resumeData      data to resume download session task from
 @param taskDelegate    the delegate to receive callback for the session task
 @param userId          userId that this task belongs to
 @param associateId     an id to associate with this session task to retrieve cache for or clean up later

 @return a background download task. Nil if already completed
 */
- (NSURLSessionDownloadTask *)backgroundDownloadTaskWithResumeData:(NSData *)resumeData taskDelegate:(id <BOXURLSessionDownloadTaskDelegate>)taskDelegate userId:(NSString *)userId associateId:(NSString *)associateId error:(NSError **)error;

/**
 Retrieve a NSURLSessionUploadTask which can be run in the background to upload file given an source file.
 If there is an existing task for userId and associateId, return that, else create a new one

 @param request         request to create upload task with
 @param fileURL         url of the source file to upload
 @param taskDelegate    the delegate to receive callback for the session task
 @param userId          userId that this task belongs to
 @param associateId     an id to associate with this session task to retrieve cache for or clean up later

 @return a background upload task. Nil if already completed
 */
- (NSURLSessionUploadTask *)backgroundUploadTaskWithRequest:(NSURLRequest *)request fromFile:(NSURL *)fileURL taskDelegate:(id <BOXURLSessionUploadTaskDelegate>)taskDelegate userId:(NSString *)userId associateId:(NSString *)associateId error:(NSError **)error;

/**
 Retrieve completed session task's cached info associated with userId and associateId

 @param userId          userId that this task belongs to
 @param associateId     an id that uniquely identify the session task for this userId
 @param error           error retrieving cached info

 @return session task's cached info
 */
- (BOXURLSessionTaskCachedInfo *)sessionTaskCompletedCachedInfoGivenUserId:(NSString *)userId associateId:(NSString *)associateId error:(NSError **)error;

/**
 Clean up session task's cached info associated with userId and associateId.
 Its task delegate will no longer handle callbacks for the task if any

 @param userId          userId that this task belongs to
 @param associateId     an id that uniquely identify the session task for this userId
 @param error           error retrieving cached info

 @return YES if successfully clean up, NO otherwise
 */
- (BOOL)cleanUpSessionTaskInfoGivenUserId:(NSString *)userId associateId:(NSString *)associateId error:(NSError **)error;

/**
 * Asynchronously calls a completion callback with all background upload, and download tasks in a session.
 */
- (void)pendingBackgroundDownloadUploadSessionTasks:(void (^)(NSArray<NSURLSessionUploadTask *> * _Nonnull uploadTasks, NSArray<NSURLSessionDownloadTask *> * _Nonnull downloadTasks))completion;

- (void)cancelAndCleanUpBackgroundSessionTasksForUserId:(NSString *)userId error:(NSError **)outError;

@end

NS_ASSUME_NONNULL_END