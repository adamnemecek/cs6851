/*
 * Partially persistent linked list.
 */

#include <algorithm>
#include <map>
#include <iostream>
#include <cstdio>
#include <vector>

typedef unsigned int uint;

template<typename T>
struct mod {
    T newVal_;
    uint version_;
    mod(T newVal, uint version) :
	newVal_(newVal),
	version_(version) {}
};

template<typename K, typename V>
class PPMap {
    class node {
    public:
	K key_;
	V value_;
	node *next_;
	node *back_;

	std::vector< mod<V> > modVal_;
	std::vector< mod<node *> > modNext_;

	node(const K &key, const V &value, node *next, node *back = NULL) :
	    key_(key),
	    value_(value),
	    next_(next),
	    back_(back) {}

	node(const node &other) :
	key_(other.key_),
	value_(other.value_),
	next_(other.next_),
	back_(other.back_) {}

	V getValue(uint version) const {
	    V ret = value_;
	    for (const auto &mod : modVal_) {
		if (mod.version_ > version)
		    break;
		ret = mod.newVal_;
	    }
	    return ret;
	}

	node *getNext(uint version) const {
	    node *ret = next_;
	    for (const auto &mod : modNext_) {
		if (mod.version_ > version)
		    break;
		ret = mod.newVal_;
	    }
	    return ret;
	}

	node *makeNew() const {
	    node *ret = new node(key_, value_, next_, back_);
	    for (const auto &mod : modVal_)
		ret->value_ = mod.newVal_;
	    for (const auto &mod : modNext_)
		ret->next_ = mod.newVal_;
	    return ret;
	}

	void updateVal(V value, uint version) {
	    if (!full()) {
		modVal_.push_back(mod<V>(value, version));
		return;
	    }
	    node *newNode = makeNew();
	    newNode->value_ = value;
	    if (newNode->next_ != NULL)
		newNode->next_->back_ = newNode;
	    if (newNode->back_ != NULL)
	    newNode->back_->updateNext(newNode, version);
	}

	void updateNext(node *next, uint version) {
	    if (!full()) {
		modNext_.push_back(mod<node *>(next, version));
		return;
	    }
	    node *newNode = makeNew();
	    newNode->next_ = next;
	    if (newNode->next_ != NULL)
		newNode->next_->back_ = newNode;
	    if (newNode->back_ != NULL)
		newNode->back_->updateNext(newNode, version);
	}

	bool full() const {
	    return modVal_.size() + modNext_.size() == 2;
	}
    };

public:
    void add(const K &key, const V &value) {
	++version_;
	if (head_.empty()) {
	    head_[version_] = new node(key, value, NULL);
	    return;
	}
	
	node *curr = getHead(version_);
	if (key < curr->key_) {
	    node *newHead = new node(key, value, curr);
	    curr->back_ = newHead;
	    head_[version_] = newHead;
	    return;
	}

	while (curr != NULL) {
	    if (curr->key_ == key) {
		curr->updateVal(value, version_);
		return;
	    }
	    node *next = curr->getNext(version_);
	    if (next == NULL || (key > curr->key_ && key < next->key_)) {
		node *newNode = new node(key, value, next, curr);
		if (next != NULL)
		    next->back_ = newNode;
		curr->updateNext(newNode, version_);
	    }
	    
	    curr = next;
	}
    }

    V search(const K &key, uint version) const {
	node *curr = getHead(version);
	while (curr != NULL) {
	    if (curr->key_ == key)
		return curr->getValue(version);
	    curr = curr->getNext(version);
	}
	return V();
    }

    node *getHead(uint version) const {
	auto it = head_.upper_bound(version);
	return (--it)->second;
    }

    uint currVersion() const {
	return version_;
    }

private:
    std::map<uint, node *> head_;
    uint version_;
};

int main(int argc, char **argv) {
    PPMap<int, int> mp;
    for (int i = 0; i < 10; ++i)
	mp.add(i, i);

    uint version = mp.currVersion();
    
    for (int i = 0; i < 20; ++i)
	mp.add(i, i + 10);

    uint version2 = mp.currVersion();

    for (int i = 0; i < 20; ++i)
	std::cout << mp.search(i, version) << " " << mp.search(i, version2) << std::endl;

    for (int i = 0; i < 50; ++i)
	mp.add(i, i + 100);

    uint version3 = mp.currVersion();
    for (int i = 0; i < 50; ++i)
	std::cout << mp.search(i, version3) << " " << mp.search(i, version2) << std::endl;

    return 0;
}
