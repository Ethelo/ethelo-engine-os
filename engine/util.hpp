#pragma once

namespace ethelo
{
    template<typename T>
    class indexed_vector
    {
        std::vector<T> vec_;
        std::unordered_map<std::string, size_t> index_;

        static const char* type_name() { return "item"; }

    public:
        indexed_vector() {};
        indexed_vector(const indexed_vector& other)
            : vec_(other.vec_), index_(other.index_) {}
        indexed_vector(indexed_vector&& other)
            : vec_(std::move(other.vec_)), index_(std::move(other.index_)) {}

        indexed_vector& operator=(const indexed_vector& other) {
            vec_ = other.vec_;
            index_ = other.index_;
            return *this;
        }

        void load(const std::vector<T>& vec)
        {
            clear();
            for (const auto& item : vec) {
                if (index_.find(item.name()) != index_.end())
                    throw std::invalid_argument(std::string("duplicate ") + type_name() + ": " + item.name());
                vec_.push_back(item);
                if (!item.name().empty())
                    index_[item.name()] = vec_.size() - 1;
            }
        }

        std::ptrdiff_t find(const std::string& name) const {
            auto iter = index_.find(name);
            return (iter != index_.end()) ? iter->second : -1;
        }

        const std::vector<T>* operator->() const { return &vec_; }
        const T& operator*() const { return vec_; }

        auto empty() const noexcept -> decltype(vec_.empty()) { return vec_.empty(); }
        void clear() { vec_.clear(); index_.clear(); } 
        auto size() const noexcept -> decltype(vec_.size()) { return vec_.size(); }
        auto front() const -> decltype(vec_.front()) { return vec_.front(); }
        auto back() const -> decltype(vec_.back()) { return vec_.back(); }
        auto operator[](typename std::vector<T>::size_type pos) const -> decltype(vec_[pos]) { return vec_[pos]; }
        auto at(typename std::vector<T>::size_type pos) const noexcept -> decltype(vec_.at(pos)) { return vec_.at(pos); }
        auto begin() const noexcept -> const decltype(vec_.begin()) { return vec_.begin(); }
        auto cbegin() const noexcept -> const decltype(vec_.cend()) { return vec_.cbegin(); }
        auto end() const noexcept -> const decltype(vec_.end()) { return vec_.end(); }
        auto cend() const noexcept -> const decltype(vec_.cend()) { return vec_.cend(); }
    };

    class option; template<> inline const char* indexed_vector<option>::type_name() { return "option"; }
    class variable; template<> inline const char* indexed_vector<variable>::type_name() { return "variable"; }
    class criterion; template<> inline const char* indexed_vector<criterion>::type_name() { return "criterion"; }
    class constraint; template<> inline const char* indexed_vector<constraint>::type_name() { return "constraint"; }
    class detail; template<> inline const char* indexed_vector<detail>::type_name() { return "detail"; }
}
