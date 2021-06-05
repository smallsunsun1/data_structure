#ifndef SKIP_LIST_
#define SKIP_LIST_

#include "concept.h"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <random>
#include <string_view>

namespace sss {

inline constexpr uint32_t kMaxHeight = 12;
const inline std::uniform_int_distribution<> kDistribute(0, kMaxHeight);

template <typename T, typename Comparator>
requires is_string_view_compator<Comparator> class SkipList {
private:
  struct Node;

public:
  SkipList(Comparator compare)
      : compare_(std::move(compare_)), head_(NewNode("", kMaxHeight)),
        max_height_(1) {
    for (uint32_t i = 0; i < kMaxHeight; ++i) {
      head_->SetNext(i, nullptr);
    }
  }
  SkipList(const SkipList &) = delete;
  SkipList &operator=(const SkipList &) = delete;
  ~SkipList() {
    while (head_) {
      auto *node = head_;
      head_ = head_->next_[0].load(std::memory_order_relaxed);
      node->~Node();
      std::free(node);
    }
  }
  void Insert(std::string_view key);
  bool Contains(std::string_view key) const;
  class Iterator {
  public:
    Iterator(const SkipList *list) : list_(list) {}
    bool Valid() const {return node_ != nullptr;}
    std::string_view Key() const {

    }
    Iterator &operator++();
    Iterator &operator--();
    void Seek(std::string_view target);
    void SeekToFirst();
    void SeekToLast();

  private:
    const SkipList *list_;
    Node *node_ = nullptr;
  };

private:
  inline uint32_t GetMaxHeight() {
    return max_height_.load(std::memory_order_relaxed);
  }
  Node *NewNode(std::string_view key, uint32_t height) const {
    void *mem =
        std::malloc(sizeof(Node) + sizeof(std::atomic<Node *>) * (height - 1));
    return new (mem) Node(key);
  }
  uint32_t RandomHeight();
  bool Equal(std::string_view a, std::string_view b) const {
    return compare_(a, b);
  }

  Comparator const compare_;
  Node *head_;
  std::atomic<uint32_t> max_height_;
  std::random_device rand_;
};

template <typename T, typename Comparator>
requires is_string_view_compator<Comparator> struct SkipList<T,
                                                             Comparator>::Node {
public:
  explicit Node(std::string_view key) : key_(key) {}
  Node *Next(uint32_t n) { return next_[n].load(std::memory_order_acquire); }
  void SetNext(uint32_t n, Node *node) {
    next_[n].store(node, std::memory_order_release);
  }
  Node *FastNext(uint32_t n) {
    return next_[n].load(std::memory_order_relaxed);
  }
  Node *FastSetNext(uint32_t n, Node *node) {
    next_[n].store(node, std::memory_order_relaxed);
  }

private:
  friend class SkipList<T, Comparator>;
  std::string_view key_;
  std::atomic<Node *> next_[1];
};

} // namespace sss

#endif /* SKIP_LIST_ */
