#ifndef SKIP_LIST_
#define SKIP_LIST_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <random>
#include <string_view>

namespace sss {

template <typename Key, typename Comparator> class SkipList {
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
  void Insert(const Key& key) {
    Node *prev[kMaxHeight];
    Node *x = FindGreaterOrEqual(key, prev);
    assert(x == nullptr || !Equal(key, x->key_));
    uint32_t height = RandomHeight();
    if (height > GetMaxHeight()) {
      for (uint32_t i = GetMaxHeight(); i < height; ++i) {
        prev[i] = head_;
      }
      max_height_.store(height, std::memory_order_relaxed);
    }
    x = NewNode(key, height);
    for (uint32_t i = 0; i < height; ++i) {
      x->FastSetNext(i, prev[i]->FastNext(i));
      prev[i]->SetNext(i, x);
    }
  }
  bool Contains(const Key& key) const {
    Node *x = FindGreaterOrEqual(key, nullptr);
    if (x != nullptr && Equal(key, x->key_)) {
      return true;
    } else {
      return false;
    }
  }
  class Iterator {
  public:
    Iterator(const SkipList *list) : list_(list) {}
    bool Valid() const { return node_ != nullptr; }
    const Key& key() const {
      assert(Valid());
      return node_->key_;
    }
    Iterator &operator++() {
      assert(Valid());
      node_ = node_->Next(0);
      return *this;
    }
    Iterator &operator--();
    void Seek(const Key& target);
    void SeekToFirst();
    void SeekToLast();

  private:
    const SkipList *list_;
    Node *node_ = nullptr;
  };

private:
  static constexpr uint32_t kMaxHeight = 12;
  inline uint32_t GetMaxHeight() const {
    return max_height_.load(std::memory_order_relaxed);
  }
  Node *NewNode(const Key& key, uint32_t height) const {
    void *mem =
        std::malloc(sizeof(Node) + sizeof(std::atomic<Node *>) * (height - 1));
    return new (mem) Node(key);
  }
  uint32_t RandomHeight() {
    static constexpr uint32_t kBranching = 4;
    uint32_t height = 1;
    while (height < kMaxHeight && ((dist_(rand_) % kBranching) == 0)) {
      height++;
    }
    return height;
  }
  bool Equal(const Key& a, const Key& b) const { return a == b; }
  bool IsAfterNode(const Key& key, Node *node) const {
    return (node != nullptr) && (compare_(node->key_, key));
  }
  Node *FindGreaterOrEqual(const Key& key, Node **prev) const {
    Node *x = head_;
    uint32_t level = GetMaxHeight() - 1;
    while (true) {
      Node *next = x->Next(level);
      if (IsAfterNode(key, next)) {
        x = next;
      } else {
        if (prev != nullptr)
          prev[level] = x;
        if (level == 0)
          return next;
        else {
          level--;
        }
      }
    }
  }
  Node *FindLessThan(const Key& key) const {
    Node *x = head_;
    uint32_t level = GetMaxHeight() - 1;
    while (true) {
      assert(x == head_ || compare_(x->key_, key) < 0);
      Node *next = x->Next(level);
      if (next == nullptr || compare_(next->key_, key) >= 0) {
        if (level == 0) {
          return x;
        } else {
          level--;
        }
      } else {
        x = next;
      }
    }
  }
  Node *FindLast() const {
    Node *x = head_;
    uint32_t level = GetMaxHeight() - 1;
    while (true) {
      Node *next = x->Next(level);
      if (next == nullptr) {
        if (level == 0) {
          return x;
        } else {
          level--;
        }
      } else {
        x = next;
      }
    }
  }

  Comparator const compare_;
  Node *head_;
  std::atomic<uint32_t> max_height_;
  std::random_device rand_;
  std::uniform_int_distribution<> dist_;
};

template <typename Key, typename Comparator>
struct SkipList<Key, Comparator>::Node {
public:
  explicit Node(const Key& key) : key_(key) {}
  Node *Next(uint32_t n) { return next_[n].load(std::memory_order_acquire); }
  void SetNext(uint32_t n, Node *node) {
    next_[n].store(node, std::memory_order_release);
  }
  Node *FastNext(uint32_t n) {
    return next_[n].load(std::memory_order_relaxed);
  }
  void FastSetNext(uint32_t n, Node *node) {
    next_[n].store(node, std::memory_order_relaxed);
  }

private:
  friend class SkipList<Key, Comparator>;
  friend class SkipList<Key, Comparator>::Iterator;
  Key key_;
  std::atomic<Node *> next_[1];
};

} // namespace sss

#endif /* SKIP_LIST_ */
