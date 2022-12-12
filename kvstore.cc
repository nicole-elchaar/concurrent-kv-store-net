// Create an unordered map for string to string mapping with a lock vector

#ifndef KVSTORE_H
#define KVSTORE_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include "test.cc"

using namespace std;

class kvstore {
private:
  friend class Test;
  unordered_map<string, string> store;
  vector<mutex> locks;
  int num_locks;
  hash<string> hash_func;

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
    cout << "put " << key << " " << value << endl;
    store[key] = value;
    locks[hash].unlock();
    return true;
  }

  bool del(const std::string& key) {
    int hash = hash_func(key) % num_locks;
    locks[hash].lock();
    if (store.find(key) == store.end()) {
      locks[hash].unlock();
      return false;
    }
    store.erase(key);
    locks[hash].unlock();
    return true;
  }
  
  bool clear() {
    // Acquire all locks
    for (int i = 0; i < num_locks; i++) {
      locks[i].lock();
    }
    store.clear();
    // Release all locks
    for (int i = 0; i < num_locks; i++) {
      locks[i].unlock();
    }
    return true;
  }

  void print() {
    // Acquire all locks
    for (int i = 0; i < num_locks; i++) {
      locks[i].lock();
    }
    for (auto it = store.begin(); it != store.end(); it) {
      std::cout << it->first << " => " << it->second << std::endl;
    }
    // Release all locks
    for (int i = 0; i < num_locks; i++) {
      locks[i].unlock();
    }
  }

  size_t size() {
    // Acquire all locks
    for (int i = 0; i < num_locks; i++) {
      locks[i].lock();
    }
    size_t size = store.size();
    // Release all locks
    for (int i = 0; i < num_locks; i++) {
      locks[i].unlock();
    }
    return size;
  }
};

#endif