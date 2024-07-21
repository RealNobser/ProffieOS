#ifndef MODE_MENU_LIST_H
#define MODE_MENU_LIST_H

// Menues made of menu entries.
// Use like:
// MenuEntryMenu<SPEC,
//    MenuEntry1,
//    MenuEntry2,
//    MenuEntry3>

#include "../common/typelist.h"

namespace mode {

struct MenuEntry {
  static const int size = 1;
  virtual void say(int entry) = 0;
  virtual void select(int entry) = 0;
};

struct NullEntry : public MenuEntry {
  static const int size = 0;
  virtual void say(int entry) {};
  virtual void select(int entry) {};
};

template<class SUBMENU, class SOUND>
struct SubMenuEntry : public MenuEntry {
  void say(int entry) override { SOUND::say(); }
  void select(int entry) override { pushMode<SUBMENU>(); }
};

// CMD and arg are expected to be ByteArray.
template<class CMD, class ARG, class SOUND>
struct CommandMenuEntry : public MenuEntry {
  void say(int entry) override { SOUND::say(); }
  void select(int entry) override {
    CommandParser::DoParse(CMD::str, ARG::str);
  }
};


template<class SOUND>
struct PopMenuEntry : public MenuEntry {
  void say(int entry) override { SOUND::say(); }
  void select(int entry) override { popMode(); }
};

template<class A, class B>
struct MenuEntryConcat : public MenuEntry {
  static const int size = A::size + B::size;
  void say(int entry) override {
    if (entry < A::size) a_.say(entry);
    else b_.say(entry - A::size);
  }
  void select(int entry) override {
    if (entry < A::size) a_.select(entry);
    else b_.select(entry - A::size);
  }
private:
  PONUA A a_;
  PONUA B b_;
};

template<class TL> struct MenuListHelper {
  typedef SplitTypeList<TL> split;
  typedef MenuEntryConcat<typename MenuListHelper<typename split::first_half>::type,
                          typename MenuListHelper<typename split::second_half>::type > type;
};

// Specialization for one Entry
template<class MenuEntry> struct MenuListHelper<TypeList<MenuEntry>>{ typedef MenuEntry type; };

// Specialization for two Entries
template<class M1, class M2> struct MenuListHelper<TypeList<M1, M2>> { typedef MenuEntryConcat<M1, M2> type; };

template<class MenuEntryTypeList> using MenuEntryList = typename MenuListHelper<MenuEntryTypeList>::type;

template<class SPEC, class MenuEntryTypeList>
class MenuEntryMenuImpl : public SPEC::MenuBase {
public:
  typedef MenuEntryTypeList MenuEntries;
  void say() override { entries_.say(this->pos_); }
  void fadeout(float len) {
    getSL<SPEC>()->fadeout(len);
  }
  uint16_t size() override { return MenuEntryTypeList::size; }
  void select() override { entries_.select(this->pos_); }
  void exit() override {
    getSL<SPEC>()->SayExit();
    popMode();
  }
private:
  PONUA MenuEntryList<MenuEntryTypeList> entries_;
};

template<class SPEC, class ... MenuEntries>
using MenuEntryMenu = MenuEntryMenuImpl<SPEC, TypeList<MenuEntries...>>;

template<class SPEC, class MENU, class ... AdditionalEntries>
using AddToMenuEntryMenu = MenuEntryMenuImpl<SPEC, ConcatTypeLists<typename MENU::MenuEntries, AdditionalEntries...>>;

}  // namespace mode

#endif // MODE_MENU_LIST_H