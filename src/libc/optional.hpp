template <typename T> class Optional {
public:
  constexpr Optional() : has_value_(false) {}

  constexpr Optional(const T &value) : has_value_(true) {
    new (&storage_) T(value);
  }

  ~Optional() { reset(); }

  constexpr bool has_value() const { return has_value_; }

  constexpr const T &value() const {
    return *reinterpret_cast<const T *>(&storage_);
  }

  constexpr T &value() { return *reinterpret_cast<T *>(&storage_); }

  void reset() {
    if (has_value_) {
      reinterpret_cast<T *>(&storage_)->~T();
      has_value_ = false;
    }
  }

private:
  bool has_value_;

  union Storage {
    char raw[sizeof(T)];
    long double align;

    constexpr Storage() : raw{} {}
  } storage_;

  // Internal cast helper
  constexpr T *ptr() { return reinterpret_cast<T *>(&storage_); }

  constexpr const T *ptr() const {
    return reinterpret_cast<const T *>(&storage_);
  }
};