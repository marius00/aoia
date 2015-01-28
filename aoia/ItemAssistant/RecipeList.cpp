#include "stdafx.h"
#include "RecipeList.h"
Combo RecipeList::getCombo(int id) const {
	for (auto it = _combos.cbegin(); it != _combos.cend(); it++)
		if (it->id == id)
			return *it;

	return Combo(0, 0, 0);
}
std::set<Item> RecipeList::getItemList(const std::list<Item>& itemsInDatabase) const {
	std::set<Item> items;
	for (auto it = _items.cbegin(); it != _items.cend(); it++)
		items.insert( *it );

	for (auto it = itemsInDatabase.cbegin(); it != itemsInDatabase.cend(); it++) {
		int id = it->id;

		// Check if this is a 'dupe item'
		auto dupe = _copies.find(ItemCopy(it->id));
		if (dupe != _copies.cend())
			id = dupe->newId;


		auto item = items.find(Item(id));

		// Regular item, just count it.
		if (item != items.cend()) {
			if (it->minql >= item->minql && it->minql <= it->maxql)
				item->nCountInDb += it->count;
		}
		else {
			std::list<unsigned int> ids = getItems(getCombo(id));
			for (auto id2 = ids.cbegin(); id2 != ids.cend(); id2++) {
				auto item = items.find( Item(*id2) );
				item->nCountInDb++;
			}
		}
	}


	return items;
}

void RecipeList::updateNames(std::map<unsigned int, std::tstring> names) {
	std::set<Item> items;	
	for (Item it : _items) {
		Item item = it;
		item.name = names[item.id];
		items.insert(item);
	}
	_items.clear();
	_items.insert(items.cbegin(), items.cend());
}
RecipeList::RecipeList(const std::tstring& recipeName, std::set<Item> items, std::set<Combo> combos, std::set<ItemCopy> copies) 
	: _name(recipeName), _items(items), _combos(combos), _copies(copies) {
}

std::tstring RecipeList::getName() const {
	return _name;
}
bool RecipeList::isValid() const {
	// Constraint: Combo items cannot be a required item
	for (auto it = _combos.cbegin(); it != _combos.cend(); it++) {
		if (_items.find( Item(it->id) ) != _items.cend())
			return false;
	}

	// Constraint: All children of a combo item must be required items (or a combo item)
	for (auto it = _combos.cbegin(); it != _combos.cend(); it++) {
		for (int i = 0; i < 2; i++) {
			if (_items.find( Item(it->childred[0]) ) == _items.cend()) {
				// Is the child a combo?
				if (_combos.find( Combo(it->childred[0]) ) == _combos.cend())
					return false;
			}
		}
	}

	for (auto item : _copies) {
		// Duplicate items can't be requirements, as they can "never exist".
		if (_items.find(Item(item.id)) != _items.cend())
			return false;

		if (_combos.find(Combo(item.id)) != _combos.cend())
			return false;
	}

	return true;
}
std::list<unsigned int> RecipeList::getDesiredItemRefs() const {
	std::list<unsigned int> refs;
	for (auto item = _items.cbegin(); item != _items.cend(); item++)
		refs.push_back(item->id);

	for (auto item = _combos.cbegin(); item != _combos.cend(); item++)
		refs.push_back(item->id);

	for (auto item : _copies)
		refs.push_back(item.id);

	return refs;
}
std::list<unsigned int> RecipeList::getItems(const Combo& combo) const {
	std::list<unsigned int> items;

	for (int i = 0; i < 2; i++) {
		auto c = _combos.find(Combo(combo.childred[i]));
		if (c == _combos.cend()) {
			items.push_back(combo.childred[i]);
		}

		else {
			auto it = getItems(*c);
			items.insert(items.end(), it.cbegin(), it.cend());
		}
	}

	return items;
}