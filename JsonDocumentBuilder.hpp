#pragma once
#include <array>
#include <string>
#include <vector>
#include <exception>
#include <functional>
#include <rapidjson/pointer.h>

namespace json_document_builder {

    using ValueType = rapidjson::Value;
    using ObjectType = rapidjson::Document;
    using DocumentType = rapidjson::Document;
    using AllocatorType = rapidjson::Document::AllocatorType;
    using BufferSizeType = std::size_t;

    template<BufferSizeType bufferSize>
    using BufferType = std::array<char, bufferSize>;

    struct InvalidFormatError : std::runtime_error {
        InvalidFormatError(std::string&& message) 
            : std::runtime_error(message) {}
    };

    struct BufferOverRunError : std::runtime_error {
        BufferOverRunError(std::string&& message)
            : std::runtime_error(message) {}
    };

    const std::function<void(const int32_t, const size_t)> formatStringValidator = 
        [](const int32_t writtenCount, const size_t bufferSize) {
            if (0 > writtenCount) {
                throw InvalidFormatError("format is invalid.");
            }

            if (bufferSize <= writtenCount) {
                std::string message("buffer size is invalid. buffer size: ");
                message.append(std::to_string(bufferSize));
                throw BufferOverRunError(std::move(message));
            }
        };

    template<typename T>
    using ValueTranslatorType = std::function<ValueType(const T&, AllocatorType&)>;

    template<typename T>
    struct DefaultValueTranslator {
        ValueType operator()(const T& value, AllocatorType& allocator) {
            return ValueType(value);
        }
    };

    template<>
    struct DefaultValueTranslator<std::string> {
        ValueType operator()(const std::string& value, AllocatorType& allocator) {
            return ValueType(value.c_str(), allocator);
        }        
    };

    template<>
    struct DefaultValueTranslator<const char*> {
        ValueType operator()(const char* value, AllocatorType& allocator) {
            return ValueType(value, allocator);
        }        
    };

    template<typename T>
    using ObjectTranslatorType = std::function<ObjectType(const T&)>;

    constexpr BufferSizeType defaultBufferSize = 1024;

    template<BufferSizeType BufferSize = defaultBufferSize>
    struct KeyReadyState;

    template<BufferSizeType BufferSize = defaultBufferSize>
    struct BaseState {
        using KeyReadyStateType = KeyReadyState<BufferSize>;
        using BufferType = BufferType<BufferSize>;

        BaseState() {}

        BaseState(DocumentType&& data)
            : data_(std::move(data)) {}

        BaseState(BaseState&& other)
            : data_(std::move(other.data_)) {}

        KeyReadyStateType at(const char* format, ...) {
            BufferType key;
            key.fill(0);

            va_list args;
            va_start(args, format);
            const int32_t writtenCount = ::vsnprintf(key.data(), key.size(), format, args);
            va_end(args);

            formatStringValidator(writtenCount, key.size());

            return KeyReadyStateType(std::move(data_), std::move(key));
        }

        DocumentType build() {
            return std::move(data_);
        }

        BaseState(const BaseState&) = delete;
        BaseState& operator=(const BaseState&) = delete;
        BaseState& operator=(BaseState&&) = delete;

    private:
        DocumentType data_;
    };

    using Builder = BaseState<>;

    template<BufferSizeType BufferSize>
    struct KeyReadyState {
        using BaseStateType = BaseState<BufferSize>;
        using BufferType = BufferType<BufferSize>;

        KeyReadyState(DocumentType&& data, BufferType&& key)
            : data_(std::move(data)), key_(std::move(key)) {}

        KeyReadyState(KeyReadyState&& other)
            : data_(std::move(other.data_)), key_(std::move(other.key_)) {}

        BaseStateType set(std::nullptr_t) {
            rapidjson::Pointer(key_.data()).Set(data_, rapidjson::Type::kNullType, data_.GetAllocator());
            return BaseStateType(std::move(data_));
        }

        template<typename T>
        BaseStateType set(T&& value) {
            rapidjson::Pointer(key_.data()).Set(data_, std::forward<T>(value), data_.GetAllocator());
            return BaseStateType(std::move(data_));
        }

        BaseStateType set(const char* format, ...) {
            BufferType value;
            value.fill(0);

            va_list args;
            va_start(args, format);
            const int32_t writtenCount = ::vsnprintf(value.data(), value.size(), format, args);
            va_end(args);

            formatStringValidator(writtenCount, value.size());

            rapidjson::Pointer(key_.data()).Set(data_, value.data(), data_.GetAllocator());
            return BaseStateType(std::move(data_));
        }

        BaseStateType set(const std::string& value) {
            rapidjson::Pointer(key_.data()).Set(data_, value.c_str(), data_.GetAllocator());
            return BaseStateType(std::move(data_));
        }

        template<typename T>
        BaseStateType set(std::vector<T>&& value, 
                          ValueTranslatorType<T>&& valueTranslator = DefaultValueTranslator<T>()) {
            return _set(value, std::move(valueTranslator));
        }

        template<typename T>
        BaseStateType set(const std::vector<T>& value, 
                          ValueTranslatorType<T>&& valueTranslator = DefaultValueTranslator<T>()) {
            return _set(value, std::move(valueTranslator));
        }

        template<typename T>
        BaseStateType set(std::vector<T>&& value,
                          ObjectTranslatorType<T>&& objectTranslator) {
            return _set(value, std::move(objectTranslator));
        }

        template<typename T>
        BaseStateType set(const std::vector<T>& value,
                          ObjectTranslatorType<T>&& objectTranslator) {
            return _set(value, std::move(objectTranslator));
        }

        KeyReadyState(const KeyReadyState&) = delete;
        KeyReadyState& operator=(const KeyReadyState&) = delete;
        KeyReadyState& operator=(KeyReadyState&&) = delete;

    private:
        template<typename T>
        BaseStateType _set(const std::vector<T>& value, 
                           ValueTranslatorType<T>&& translator) {
            BufferType key;

            for (int32_t i = 0; i < value.size(); ++i) {
                key.fill(0);

                const int32_t writtenCount = ::snprintf(key.data(), key.size(), "%s/%d", key_.data(), i);
                formatStringValidator(writtenCount, key.size());

                rapidjson::Pointer(key.data()).Set(data_, translator(value.at(i), data_.GetAllocator()), data_.GetAllocator());
            }
            return BaseStateType(std::move(data_));
        }

        template<typename T>
        BaseStateType _set(const std::vector<T>& value,
                           ObjectTranslatorType<T>&& objectTranslator) {
            BufferType key;

            for (int32_t i = 0; i < value.size(); ++i) {
                key.fill(0);

                const int32_t writtenCount = ::snprintf(key.data(), key.size(), "%s/%d", key_.data(), i);
                formatStringValidator(writtenCount, key.size());

                rapidjson::Pointer(key.data()).Set(data_, objectTranslator(value.at(i)), data_.GetAllocator());
            }
            return BaseStateType(std::move(data_));
        }

        DocumentType data_;
        BufferType key_;    
    };
} // namespace json_document_builder
