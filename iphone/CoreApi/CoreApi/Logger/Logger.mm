#import "Logger.h"
#import <OSLog/OSLog.h>

#include "base/logging.hpp"

@interface Logger ()

@property (nullable, nonatomic) NSString * filePath;
@property (nullable, nonatomic) NSFileHandle * fileHandle;
@property (nonnull, nonatomic) os_log_t osLogger;
@property (nonatomic) uint64_t fileSize;
@property (class, readonly, nonatomic) dispatch_queue_t fileLoggingQueue;

+ (Logger *)logger;
+ (void)enableFileLogging;
+ (void)disableFileLogging;
+ (void)logMessageWithLevel:(base::LogLevel)level src:(base::SrcPoint const &)src message:(std::string const &)message;
+ (base::LogLevel)baseLevel:(LogLevel)level;

- (BOOL)isFileLoggingEnabled;

@end

// Subsystem and category are used for the OSLog.
NSString * const kLoggerSubsystem = [[NSBundle mainBundle] bundleIdentifier];
NSString * const kLoggerCategory = @"OM";
NSString * const kLogFileName = @"Log.log";
NSUInteger const kMaxLogFileSize = 1024 * 1024 * 3; // 3 MB;

@implementation Logger

+ (void)initialize
{
  if (self == [Logger class]) {
    SetLogMessageFn(&LogMessage);
    SetAssertFunction(&AssertMessage);
  }
}

+ (Logger *)logger {
  static Logger * logger = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    logger = [[self alloc] init];
  });
  return logger;
}

+ (dispatch_queue_t)fileLoggingQueue {
  static dispatch_queue_t fileLoggingQueue = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    dispatch_queue_attr_t attributes = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_UTILITY, 0);
    fileLoggingQueue = dispatch_queue_create("app.organicmaps.fileLoggingQueue", attributes);
  });
  return fileLoggingQueue;
}

- (instancetype)init {
  self = [super init];
  if (self) {
    _fileSize = 0;
    _osLogger = os_log_create(kLoggerCategory.UTF8String, kLoggerSubsystem.UTF8String);
  }
  return self;
}

// MARK: - Public

+ (void)setFileLoggingEnabled:(BOOL)fileLoggingEnabled {
  fileLoggingEnabled ? [self enableFileLogging] : [self disableFileLogging];
}

+ (void)log:(LogLevel)level message:(NSString *)message {
  LOG_SHORT([self baseLevel:level], (message.UTF8String));
}

+ (BOOL)canLog:(LogLevel)level {
  return [Logger baseLevel:level] >= base::g_LogLevel;
}

+ (nullable NSURL *)getLogFileURL {
  Logger * logger = [self logger];
  return [logger isFileLoggingEnabled] ? [NSURL fileURLWithPath:logger.filePath] : nil;
}

+ (uint64_t)getLogFileSize {
  return [self logger].fileSize;
}

// MARK: - C++ injection

void LogMessage(base::LogLevel level, base::SrcPoint const & src, std::string const & message)
{
  [Logger logMessageWithLevel:level src:src message:message];
  CHECK_LESS(level, base::g_LogAbortLevel, ("Abort. Log level is too serious", level));
}

bool AssertMessage(base::SrcPoint const & src, std::string const & message)
{
  [Logger logMessageWithLevel:base::LCRITICAL src:src message:message];
  return true;
}

// MARK: - Private

+ (void)enableFileLogging {
  Logger * logger = [self logger];
  NSFileManager * fileManager = [NSFileManager defaultManager];

  // Create a log file if it doesn't exist and setup file handle for writing.
  NSString * filePath = [[fileManager temporaryDirectory] URLByAppendingPathComponent:kLogFileName].path;
  if (![fileManager fileExistsAtPath:filePath])
    [fileManager createFileAtPath:filePath contents:nil attributes:nil];

  NSFileHandle * fileHandle = [NSFileHandle fileHandleForWritingAtPath:filePath];
  if (fileHandle == nil) {
    LOG(LERROR, ("Failed to open log file for writing", filePath.UTF8String));
    [self setFileLoggingEnabled:NO];
    return;
  }
  // Clean up the file if it exceeds the maximum size.
  if ([fileManager contentsAtPath:filePath].length > kMaxLogFileSize)
    [fileHandle truncateFileAtOffset:0];

  logger.filePath = filePath;
  logger.fileHandle = fileHandle;
  LOG(LINFO, ("File logging is enabled"));
}

+ (void)disableFileLogging {
  Logger * logger = [self logger];

  [logger.fileHandle closeFile];
  logger.fileHandle = nil;
  logger.fileSize = 0;
  NSError * error;
  [NSFileManager.defaultManager removeItemAtPath:logger.filePath error:&error];
  if (error)
    LOG(LERROR, ("Failed to remove log file", logger.filePath.UTF8String));
  LOG(LINFO, ("File logging is disabled"));
}

- (BOOL)isFileLoggingEnabled {
  return self.fileHandle != nil;
}

+ (void)logMessageWithLevel:(base::LogLevel)level src:(base::SrcPoint const &)src message:(std::string const &)message {
  // Build the log message string.
  auto & logHelper = base::LogHelper::Instance();
  std::ostringstream output;
  logHelper.WriteProlog(output, level);
  logHelper.WriteLog(output, src, message);
  NSString * logMessage = [NSString stringWithUTF8String:output.str().c_str()];

  Logger * logger = [self logger];
  // Log the message into the system log.
  os_log(logger.osLogger, "%{public}s", logMessage.UTF8String);

  dispatch_async([self fileLoggingQueue], ^{
    // Write the log message into the file.
    NSFileHandle * fileHandle = logger.fileHandle;
    if (fileHandle != nil) {
      [fileHandle seekToEndOfFile];
      [fileHandle writeData:[logMessage dataUsingEncoding:NSUTF8StringEncoding]];
      logger.fileSize = [fileHandle offsetInFile];
    }
  });
}

+ (base::LogLevel)baseLevel:(LogLevel)level {
  switch (level) {
    case LogLevelDebug: return LDEBUG;
    case LogLevelInfo: return LINFO;
    case LogLevelWarning: return LWARNING;
    case LogLevelError: return LERROR;
    case LogLevelCritical: return LCRITICAL;
  }
}

@end
