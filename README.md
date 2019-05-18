# pCp - A directory copying utility
A directory copying utility that creates a new thread to copy each subdirectory/file to perform the overall task in parallel.
Includes a worker thread pool implementation (pthread) for producer-consumer problem, and a buffer implementation by using circular array as a synchronization point.

Usage: : pCp [NumberOfConsumers] [BufferSize] [SourceDirectory] [DestinationDirectory]
