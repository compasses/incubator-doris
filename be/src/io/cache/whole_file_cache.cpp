// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "io/cache/whole_file_cache.h"

#include "io/fs/local_file_system.h"

namespace doris {
namespace io {

const static std::string WHOLE_FILE_CACHE_NAME = "WHOLE_FILE_CACHE";
const static std::string WHOLE_FILE_CACHE_DONE_NAME = "WHOLE_FILE_CACHE_DONE";

WholeFileCache::WholeFileCache(const Path& cache_dir, int64_t alive_time_sec,
                               io::FileReaderSPtr remote_file_reader)
        : _cache_dir(cache_dir),
          _alive_time_sec(alive_time_sec),
          _remote_file_reader(remote_file_reader),
          _last_match_time(time(nullptr)),
          _cache_file_reader(nullptr) {}

WholeFileCache::~WholeFileCache() {}

Status WholeFileCache::read_at(size_t offset, Slice result, size_t* bytes_read) {
    if (_cache_file_reader == nullptr) {
        RETURN_IF_ERROR(_generate_cache_reader(offset, result.size));
    }
    std::shared_lock<std::shared_mutex> rlock(_cache_lock);
    RETURN_NOT_OK_STATUS_WITH_WARN(
            _cache_file_reader->read_at(offset, result, bytes_read),
            fmt::format("Read local cache file failed: {}", _cache_file_reader->path().native()));
    if (*bytes_read != result.size) {
        LOG(ERROR) << "read cache file failed: " << _cache_file_reader->path().native()
                   << ", bytes read: " << bytes_read << " vs required size: " << result.size;
        return Status::OLAPInternalError(OLAP_ERR_OS_ERROR);
    }
    _last_match_time = time(nullptr);
    return Status::OK();
}

Status WholeFileCache::_generate_cache_reader(size_t offset, size_t req_size) {
    std::lock_guard<std::shared_mutex> wrlock(_cache_lock);
    Path cache_file = _cache_dir / WHOLE_FILE_CACHE_NAME;
    Path cache_done_file = _cache_dir / WHOLE_FILE_CACHE_DONE_NAME;
    bool done_file_exist = false;
    RETURN_NOT_OK_STATUS_WITH_WARN(
            io::global_local_filesystem()->exists(cache_done_file, &done_file_exist),
            "Check local cache done file exist failed.");
    if (!done_file_exist) {
        bool cache_file_exist = false;
        RETURN_NOT_OK_STATUS_WITH_WARN(
                io::global_local_filesystem()->exists(cache_file, &cache_file_exist),
                "Check local cache file exist failed.");
        if (cache_file_exist) {
            RETURN_NOT_OK_STATUS_WITH_WARN(io::global_local_filesystem()->delete_file(cache_file),
                                           "Check local cache file exist failed.");
        }
        LOG(INFO) << "Download cache file from remote file: "
                  << _remote_file_reader->path().native() << " -> " << cache_file.native();
        std::unique_ptr<char[]> file_buf(new char[_remote_file_reader->size()]);
        Slice file_slice(file_buf.get(), _remote_file_reader->size());
        size_t bytes_read = 0;
        RETURN_NOT_OK_STATUS_WITH_WARN(
                _remote_file_reader->read_at(0, file_slice, &bytes_read),
                fmt::format("read remote file failed. {}", _remote_file_reader->path().native()));
        if (bytes_read != _remote_file_reader->size()) {
            LOG(ERROR) << "read remote file failed: " << _remote_file_reader->path().native()
                       << ", bytes read: " << bytes_read
                       << " vs file size: " << _remote_file_reader->size();
            return Status::OLAPInternalError(OLAP_ERR_OS_ERROR);
        }
        io::FileWriterPtr file_writer;
        RETURN_NOT_OK_STATUS_WITH_WARN(
                io::global_local_filesystem()->create_file(cache_file, &file_writer),
                fmt::format("Create local cache file failed: {}", cache_file.native()));
        RETURN_NOT_OK_STATUS_WITH_WARN(
                file_writer->append(file_slice),
                fmt::format("Write local cache file failed: {}", cache_file.native()));
        RETURN_NOT_OK_STATUS_WITH_WARN(
                io::global_local_filesystem()->create_file(cache_done_file, &file_writer),
                fmt::format("Create local done file failed: {}", cache_done_file.native()));
    }
    RETURN_IF_ERROR(io::global_local_filesystem()->open_file(cache_file, &_cache_file_reader));
    _cache_file_size = _cache_file_reader->size();
    _last_match_time = time(nullptr);
    LOG(INFO) << "Create cache file from remote file successfully: "
              << _remote_file_reader->path().native() << " -> " << cache_file.native();
    return Status::OK();
}

Status WholeFileCache::clean_timeout_cache() {
    if (time(nullptr) - _last_match_time > _alive_time_sec) {
        _clean_cache_internal();
    }
    return Status::OK();
}

Status WholeFileCache::clean_all_cache() {
    _clean_cache_internal();
    return Status::OK();
}

Status WholeFileCache::_clean_cache_internal() {
    std::lock_guard<std::shared_mutex> wrlock(_cache_lock);
    _cache_file_reader.reset();
    _cache_file_size = 0;
    Path cache_file = _cache_dir / WHOLE_FILE_CACHE_NAME;
    Path done_file = _cache_dir / WHOLE_FILE_CACHE_DONE_NAME;
    bool done_file_exist = false;
    RETURN_NOT_OK_STATUS_WITH_WARN(
            io::global_local_filesystem()->exists(done_file, &done_file_exist),
            "Check local done file exist failed.");
    if (done_file_exist) {
        RETURN_NOT_OK_STATUS_WITH_WARN(
                io::global_local_filesystem()->delete_file(done_file),
                fmt::format("Delete local done file failed: {}", done_file.native()));
    }
    bool cache_file_exist = false;
    RETURN_NOT_OK_STATUS_WITH_WARN(
            io::global_local_filesystem()->exists(cache_file, &cache_file_exist),
            "Check local cache file exist failed.");
    if (cache_file_exist) {
        RETURN_NOT_OK_STATUS_WITH_WARN(
                io::global_local_filesystem()->delete_file(cache_file),
                fmt::format("Delete local cache file failed: {}", cache_file.native()));
    }
    LOG(INFO) << "Delete local cache file successfully: " << cache_file.native();
    return Status::OK();
}

} // namespace io
} // namespace doris
