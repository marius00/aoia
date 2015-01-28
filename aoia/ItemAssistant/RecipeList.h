#pragma once
#include <Shared/UnicodeSupport.h>
#include <string>
#include <set>
#include <list>
#include <map>
struct Item {
	unsigned int count;
	unsigned int id;
	unsigned int minql;
	unsigned int maxql;
	mutable std::tstring name;

	Item() : nCountInDb(0) {}
	Item(unsigned int _id) : id(_id), nCountInDb(0), count(1) {}
	Item(unsigned int _id, std::tstring name_) : id(_id), nCountInDb(0), name(name_), count(1) {}

	bool Item::operator==(const Item &other) const {
		return id == other.id;
	}
	bool Item::operator<(const Item &other) const {
		return id < other.id;
	}
	mutable std::tstring comments;
	mutable unsigned int nCountInDb;
};

struct ItemCopy {
	unsigned int id; // ID of item
	unsigned int newId; // New ID

	ItemCopy() {}
	ItemCopy(unsigned int id_) : id(id_) {}

	bool ItemCopy::operator==(const ItemCopy &other) const {
		return id == other.id;
	}
	bool ItemCopy::operator<(const ItemCopy &other) const {
		return id < other.id;
	}
};


struct Combo {
	unsigned int id;
	unsigned int childred[2];
	unsigned int minql;
	unsigned int maxql;

	bool Combo::operator==(const Combo &other) const {
		return id == other.id;
	}
	bool Combo::operator<(const Combo &other) const {
		return id < other.id;
	}

	Combo() {}
	Combo(unsigned int _id) : id(_id) {}
	Combo(unsigned int _id, unsigned int c1, unsigned int c2) : id(_id) {
		childred[0] = c1;
		childred[1] = c2;
	}
};


class RecipeList {
public:
	RecipeList(const std::tstring& recipeName, std::set<Item> items, std::set<Combo> combos, std::set<ItemCopy> copies);

	// Get item IDs to search the database for
	std::list<unsigned int> getDesiredItemRefs() const;

	// Takes the items in database and returns a list of items to display to the user
	std::set<Item> getItemList(const std::list<Item>& itemsInDatabase) const;
	
	bool isValid() const;
	std::tstring getName() const;
	void updateNames(std::map<unsigned int, std::tstring> names);
private:
	std::tstring _name;
	std::set<Item> _items;	
	std::set<Combo> _combos;
	std::set<ItemCopy> _copies;

	std::list<unsigned int> getItems(const Combo& combo) const;
	Combo getCombo(int id) const;
};