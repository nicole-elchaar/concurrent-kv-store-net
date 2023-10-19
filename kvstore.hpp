/* 
 * Nicole ElChaar, CSE 411, Fall 2022
 *
 * This file contains the implementation of a key-value store with a fixed
 * number of locks.
 */

#ifndef KVSTORE_H
#define KVSTORE_H

#include "test.cc"

#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

class kvstore {
private:
  friend class Test;
  int num_locks;
  std::vector<std::mutex> locks;
  std::hash<std::string> hash_func;
  std::unordered_map<std::string, std::string> store;

public:
  kvstore(int num_locks = 100) : num_locks(num_locks), locks(num_locks) {
    hash_func = std::hash<std::string>{};
  }

  bool get(const std::string& key, std::string& value) {
    int hash = hash_func(key) % num_locks;
    locks[hash].lock();
    if (store.find(key) == store.end()) {
      locks[hash].unlock();
      return false;
    }
    value = store[key];
    locks[hash].unlock();
    return true;
  }

  bool put(const std::string& key, const std::string& value) {
    int hash = hash_func(key) % num_locks;
    locks[hash].lock();
    store[key] = value;
    locks[hash].unlock();
    return true;
  }

  bool del(const std::string& key) {
    std::cout << "Deleting " << key << std::endl;
    int hash = hash_func(key) % num_locks;
    std::cout << "Hash: " << hash << std::endl;
    locks[hash].lock();
    std::cout << "Lock acquired" << std::endl;
    if (store.find(key) == store.end()) {
      std::cout << "Key not found" << std::endl;
      locks[hash].unlock();
      std::cout << "Lock released" << std::endl;
      return false;
    }
    std::cout << "Key found" << std::endl;
    store.erase(key);
    std::cout << "Key erased" << std::endl;
    locks[hash].unlock();
    std::cout << "Lock released" << std::endl;
    return true;
  }
  
  bool clear() {
    for (int i = 0; i < num_locks; i++) {
      locks[i].lock();    // Acquire all locks
    }
    store.clear();
    if (store.size() != 0) {
      for (int i = 0; i < num_locks; i++) {
        locks[i].unlock();  // Release all locks
      }
      return false;
    }
    for (int i = 0; i < num_locks; i++) {
      locks[i].unlock();  // Release all locks
    }
    return true;
  }

  void print() {
    for (int i = 0; i < num_locks; i++) {
      locks[i].lock();    // Acquire all locks
    }
    for (auto it = store.begin(); it != store.end(); it++) {
      std::cout << it->first << " => " << it->second << std::endl;
    }
    for (int i = 0; i < num_locks; i++) {
      locks[i].unlock();  // Release all locks
    }
  }

  size_t size() {
    for (int i = 0; i < num_locks; i++) {
      locks[i].lock();    // Acquire all locks
    }
    size_t size = store.size();
    for (int i = 0; i < num_locks; i++) {
      locks[i].unlock();  // Release all locks
    }
    return size;
  }
};

#endif
