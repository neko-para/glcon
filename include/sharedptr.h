#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_

template <typename Type>
class SharedPtr {
	struct Node {
		Type* value;
		int ref;
		Node(Type* p) : value(p), ref(0) {}
		~Node() {
			if (value) {
				delete value;
			}
		}
	};
	Node* node;

	void Take(Node* n) {
		node = n;
		++node->ref;
	}
	void Drop() {
		--node->ref;
		if (!node->ref) {
			delete node;
		}
	}
public:
	explicit SharedPtr(Type* p = 0) {
		Take(new Node(p));
	}
	SharedPtr(const SharedPtr& p) {
		Take(p.node);
	}
	~SharedPtr() {
		Drop();
	}
	const SharedPtr& operator=(const SharedPtr& p) {
		if (&p == this) {
			return *this;
		}
		Drop();
		Take(p.node);
		return *this;
	}
	Type& operator*() const {
		return *node->value;
	}
	Type* operator->() const {
		return node->value;
	}
};

template <typename Type>
inline bool operator<(const Type& a, const Type& b) {
	return *a < *b;
}

#endif