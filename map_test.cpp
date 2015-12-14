
// I N C L U D E S ////////////////////////////////////////////////////////////

#include <iostream>

#include "safemap.h"

// F U N C T I O N S //////////////////////////////////////////////////////////

// Recommendation: run this code with different levels of optimization and both
// inside and out of valgrind. These sort of things can change thread handoff
// timing and reveal errors that are unlikely or impossible in other conditions.

#if 1
typedef int MyValue;
#else
class MyValue 
{
public:
  MyValue() : foo(0) {};
  MyValue(int _foo) : foo(_foo) {};
  operator int() const { return foo; };

  int foo;
};
#endif

void seeker_thread(const safe::map<int, MyValue>& map)
{
#ifdef DEBUG
 { char buf[512]; sprintf(buf, "seeker_thread: %lu", map.size()); safe::debug(buf); }
#endif

  static int x = 0;

  auto iter = map.cbegin();
  for (int i = 0; i < 1000; ++i)
  {
    int choice = rand() % 1000;

    if (choice < 200)
    {
      DEBUG_ITER2;
      try
        { x += map.at(1); }
      catch (...)
        {} 
    }
    else if (choice < 300)
    {
      DEBUG_ITER2;
      auto ret = map.equal_range(1);
      iter = ret.first;
    }
    else if (choice < 600)
    {
      DEBUG_ITER2;
      iter = map.find(1);
    }
    else if (choice < 800)
    {
      DEBUG_ITER2;
      iter = map.lower_bound(10);
    }
    else
    {
      DEBUG_ITER2;
      iter = map.upper_bound(10);
    }
  }
}

void seeker_changer_thread(safe::map<int, MyValue>& map)
{
#ifdef DEBUG
 { char buf[512]; sprintf(buf, "seeker_changer_thread: %lu", map.size()); safe::debug(buf); }
#endif

  static int x = 0;

  auto iter = map.begin();
  DEBUG_ITER2;
  for (int i = 0; i < 1000; ++i)
  {
    int choice = rand() % 1000;
  
    if (choice < 80)
    {
      DEBUG_ITER2;
      try
        { map.at(1) += x; }
      catch (...)
        {} 
    }
    else if (choice < 160)
    {
      DEBUG_ITER2;
      map.emplace((rand() % 100000) / (rand() % 100 + 1), MyValue(rand()));
    }
    else if (choice < 250)
    {
      DEBUG_ITER2;
      auto ret = map.equal_range(100);
      if (ret.second != map.end())
        ret.second->second = MyValue(rand());
    }
    else if (choice < 350)
    {
      DEBUG_ITER2;
      iter = map.find(1);
      if (iter != map.end())
        iter->second = MyValue(rand());
    }
    else if (choice < 450)
    {
      DEBUG_ITER2;
      map.insert(std::make_pair((rand() % 100000) / (rand() % 100 + 1), MyValue(rand())));
    }
    else if (choice < 550)
    {
      DEBUG_ITER2;
      auto ret = std::make_pair((rand() % 100000) / (rand() % 100 + 1), MyValue(rand()));
      map.insert(ret);
    }
    else if (choice < 600)
    {
      DEBUG_ITER2;
      std::vector<std::pair<int, MyValue>> ret;
      for (int j = 0; j < rand() % 10; ++j)
        ret.push_back(std::make_pair((rand() % 100000) / (rand() % 100 + 1), MyValue(rand())));
      map.insert(ret.begin(), ret.end());
    }
    else if (choice < 700)
    {
      DEBUG_ITER2;
      iter = map.lower_bound(rand() % 10000);
      if (iter != map.end())
        map.erase(iter);
    }
    else if (choice < 800)
    {
      DEBUG_ITER2;
      iter = map.upper_bound(rand() % 10000);
      if (iter != map.end())
        map.erase(iter);
    }
    else if (choice < 995)
    {
      safe::map<int, MyValue>* map2; 
      const int choice2 = rand() % 9;
      switch (choice2)
      {
        case 0:
          { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(); break; }
        case 1:
          { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(map); break; }
        case 2:
          { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(); break; }
        case 3:
          { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(std::move(map)); break; }
        case 4:
          { DEBUG_ITER2; map2 = new safe::map<int, MyValue>({{11, MyValue(11)}}); break; }
        default:
        {
          std::map<int, MyValue> map3; 
          while (true)
          {
            map3.emplace((rand() % 100000) / (rand() % 100 + 1), MyValue(rand()));
            if (!(rand() % 50))
              break;
          }
          if (choice2 == 5)
            { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(map3); }
          else if (choice2 == 6)
            { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(std::move(map3)); }
          else if (choice2 == 7)
            { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(map3.lower_bound(100), map3.upper_bound(10000)); }
          else
            { DEBUG_ITER2; map2 = new safe::map<int, MyValue>(); map2->swap(map3); }
          usleep(50);
          DEBUG_ITER2;
          break;
        }
      }
      while (true)
      {
        map2->emplace((rand() % 100000) / (rand() % 100 + 1), MyValue(rand()));
        if (!(rand() % 50))
          break;
      }
      switch (rand() % 4)
      {
        case 0:
          { DEBUG_ITER2; map.swap(*map2); break; }
        case 1:
          { DEBUG_ITER2; map = *map2; break; }
        case 2:
          { DEBUG_ITER2; map = std::move(*map2); break; }
        case 3:
          { DEBUG_ITER2; map.insert(map2->lower_bound(20), map2->upper_bound(50000)); break; }
      }
      usleep(50);
      DEBUG_ITER2;
      delete map2;
      DEBUG_ITER2;
    }
    else if (choice < 997)
    {
      DEBUG_ITER2;
      map[(rand() % 100000) / (rand() % 100 + 1)] = MyValue(rand());
    }
    else if (choice < 998)
    {
      DEBUG_ITER2;
      map.clear();
      while (true)
      {
        map.emplace((rand() % 100000) / (rand() % 100 + 1), MyValue(rand()));
        if (!(rand() % 50))
          break;
      }
    }
    else
    {
      DEBUG_ITER2;
      map.erase(rand() % 10000);
    }

    while (map.size() > 100000)
      map.erase(map.begin());

    DEBUG_ITER2;
  }
}

void scanner_thread(const safe::map<int, MyValue>& map)
{
#ifdef DEBUG
 { char buf[512]; sprintf(buf, "scanner_thread: %lu", map.size()); safe::debug(buf); }
#endif

  static int x = 0;
  
  for (auto iter = map.cbegin(); iter != map.cend(); ++iter)
  {
    int choice = rand() % 1000;

    if (choice < 200)
    {
      DEBUG_ITER2;
      while (iter != map.cbegin())
      {
        DEBUG_ITER2;
        --iter;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 400)
    {
      DEBUG_ITER2;
      while (iter != map.cbegin())
      {
        DEBUG_ITER2;
        iter--;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 600)
    {
      DEBUG_ITER2;
      while (iter != map.cend())
      {
        DEBUG_ITER2;
        ++iter;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.cend())
        break;
    }
    else if (choice < 800)
    {
      DEBUG_ITER2;
      while (iter != map.cend())
      {
        DEBUG_ITER2;
        iter++;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.cend())
        break;
    }
    else
    {
        DEBUG_ITER2;
      for (int i = 0; i < 100; i++)
        x += rand() * iter->second;
    }

    DEBUG_ITER2;
  }
}

void reverse_scanner_thread(const safe::map<int, MyValue>& map)
{
#ifdef DEBUG
 { char buf[512]; sprintf(buf, "reverse_scanner_thread: %lu", map.size()); safe::debug(buf); }
#endif

  static int x = 0;
  
  for (auto iter = map.crbegin(); iter != map.crend(); ++iter)
  {
    int choice = rand() % 1000;

    if (choice < 200)
    {
      DEBUG_RITER2;
      while (iter != map.crbegin())
      {
        DEBUG_RITER2;
        --iter;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 400)
    {
      DEBUG_RITER2;
      while (iter != map.crbegin())
      {
        DEBUG_RITER2;
        iter--;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 600)
    {
      DEBUG_RITER2;
      while (iter != map.crend())
      {
        DEBUG_RITER2;
        ++iter;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.crend())
        break;
    }
    else if (choice < 800)
    {
      DEBUG_RITER2;
      while (iter != map.crend())
      {
        DEBUG_RITER2;
        iter++;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.crend())
        break;
    }
    else
    {
      DEBUG_RITER2;
      for (int i = 0; i < 100; i++)
        x += rand() * iter->second;
    }

    DEBUG_RITER2;
  }
}

void scan_changer_thread(safe::map<int, MyValue>& map)
{
#ifdef DEBUG
 { char buf[512]; sprintf(buf, "scan_changer_thread: %lu", map.size()); safe::debug(buf); }
#endif

  for (auto iter = map.begin(); iter != map.end(); ++iter)
  {
    int choice = rand() % 1000;

    if (choice < 150)
    {
      DEBUG_ITER2;
      while (iter != map.begin())
      {
        DEBUG_ITER2;
        --iter;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 300)
    {
      DEBUG_ITER2;
      while (iter != map.begin())
      {
        DEBUG_ITER2;
        iter--;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 450)
    {
      DEBUG_ITER2;
      while (iter != map.end())
      {
        DEBUG_ITER2;
        ++iter;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.end())
        break;
    }
    else if (choice < 600)
    {
      DEBUG_ITER2;
      while (iter != map.end())
      {
        DEBUG_ITER2;
        iter++;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.end())
        break;
    }
    else if (choice < 700)
    {
      DEBUG_ITER2;
      map.emplace_hint(iter, (rand() % 100000) / (rand() % 100 + 1), MyValue(rand()));
    }
    else if (choice < 800)
    {
      DEBUG_ITER2;
      map.insert(iter, std::make_pair((rand() % 100000) / (rand() % 100 + 1), MyValue(rand())));
    }
    else if (choice < 900)
    {
      DEBUG_ITER2;
      map[(rand() % 100000) / (rand() % 100 + 1)] = MyValue(rand());
    }
    else
    {
      DEBUG_ITER2;
      auto last = iter;
      DEBUG_ITER2;
      while (last != map.begin())
      {
        DEBUG_ITER2;
        --last;
        DEBUG_ITER2;
        if (!(rand() % 10))
          break;
      }
      map.erase(last, iter);
    }

    while (map.size() > 100000)
      map.erase(map.begin());

    DEBUG_ITER2;
  }

  for (auto iter = map.rbegin(); iter != map.rend(); ++iter)
  {
    int choice = rand() % 1000;

    if (choice < 150)
    {
      DEBUG_RITER2;
      while (iter != map.rbegin())
      {
        DEBUG_RITER2;
        --iter;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 300)
    {
      DEBUG_RITER2;
      while (iter != map.rbegin())
      {
        DEBUG_RITER2;
        iter--;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
    }
    else if (choice < 450)
    {
      DEBUG_RITER2;
      while (iter != map.rend())
      {
        DEBUG_RITER2;
        ++iter;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.rend())
        break;
    }
    else if (choice < 600)
    {
      DEBUG_RITER2;
      while (iter != map.rend())
      {
        DEBUG_RITER2;
        iter++;
        DEBUG_RITER2;
        if (!(rand() % 10))
          break;
      }
      if (iter == map.rend())
        break;
    }
    else if (choice < 700)
    {
      DEBUG_RITER2;
      map.insert(std::make_pair((rand() % 100000) / (rand() % 100 + 1), MyValue(rand())));
    }
    else if (choice < 800)
    {
      DEBUG_RITER2;
      map.insert({(rand() % 100000) / (rand() % 100 + 1), MyValue(rand())});
    }
    else
    {
      DEBUG_RITER2;
      map.erase(random() % 10000);
    }

    while (map.size() > 100000)
      map.erase(map.begin());

    DEBUG_RITER2;
  }
}


template<bool Reversed, safe::IterationType Iteration, bool Circular>
void iteration_test()
{
  std::string s = "##########    Testing reversed=" + std::to_string(Reversed) + " iteration type=" + (Iteration == safe::OnlyForward ? "OnlyForward" : Iteration == safe::ForwardThenBackward ? "ForwardThenBackward" : Iteration == safe::ForwardSameThenBackward ? "ForwardSameThenBackward" : "EvenErased") + " circular=" + std::to_string(Circular) + "\n";
  std::cout << s;

  safe::map<int, MyValue, Circular, Iteration> map;

  for (int i = 1; i <= 4; ++i)
    map.emplace(i, MyValue(i));

  // safe::map doesn't have a factory method for making iterators of arbitrary-reverseness, so let's spell it out...
  map.m_lock->lock();
  typename safe::map<int, MyValue, Circular, Iteration>::template iterator_base<typename safe::map<int, MyValue, Circular, Iteration>::base_iterator, Reversed> iter1(map.m_map, map.m_lock), iter2(map.m_map, map.m_lock);
  map.m_lock->unlock();
  
  iter1 = map.find(Reversed ? 2 : 3);
  iter2 = map.find(Reversed ? 1 : 4);
  map.erase(map.find(Reversed ? 2 : 3));

  if ((!Reversed) && (Iteration != safe::EvenErased))
    std::cout << "##########    1. The next non-debug line should read: >>> 1 2 4 " << std::endl;
  else if ((Reversed) && (Iteration != safe::EvenErased))
    std::cout << "##########    1. The next non-debug line should read: >>> 1 3 4 " << std::endl;
  else if (Iteration == safe::EvenErased)
    std::cout << "##########    1. The next non-debug line should read: >>> 1 2 3 4 " << std::endl;

  s = ">>> ";
  if (Circular)
  {
    for (auto i = map.begin(); ; )
    {
      s += std::to_string(i->first) + " ";
      ++i;
      if (i == map.begin())
        break;
    }
  }
  else if (Iteration == safe::ForwardSameThenBackward)
  {
    for (auto i = map.begin(), last = map.end(); i != map.end() && i != last; last = i, last = i++)
      s += std::to_string(i->first) + " ";
  }
  else if (Iteration == safe::ForwardThenBackward)
  {
    for (auto i = map.begin(), last = map.end(); i != map.end() && (last == map.end() || i->first > last->first); last = i, last = i++)
      s += std::to_string(i->first) + " ";
  }
  else
  {
    for (auto i = map.begin(); i != map.end(); ++i)
      s += std::to_string(i->first) + " ";
  }
  s += "\n";
  std::cout << s;


  if ((Iteration == safe::OnlyForward) && (!Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> *" << std::endl;
  else if ((!Reversed) && (Iteration == safe::OnlyForward) && (Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> 1" << std::endl;
  else if ((Reversed) && (Iteration == safe::OnlyForward) && (Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> 4" << std::endl;
  else if ((!Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (!Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> 2" << std::endl;
  else if ((Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (!Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> 3" << std::endl;
  else if ((!Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> 1" << std::endl;
  else if ((Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (Circular))
    std::cout << "##########    2. The next non-debug line should read: >>> 4" << std::endl;
  else if ((!Reversed) && (Iteration == safe::EvenErased))
    std::cout << "##########    2. The next non-debug line should read: >>> 4" << std::endl;
  else if ((Reversed) && (Iteration == safe::EvenErased))
    std::cout << "##########    2. The next non-debug line should read: >>> 1" << std::endl;

  map.erase(map.find(Reversed ? 1 : 4));
  ++iter1;

  s = ">>> ";
  if (iter1 == map.end())
    s += "*\n";
  else
    s += std::to_string(iter1->first) + "\n";
  std::cout << s;

  if ((!Reversed) && (Iteration == safe::OnlyForward))
    std::cout << "##########    3. The next non-debug line should read: >>> 2" << std::endl;
  else if ((Reversed) && (Iteration == safe::OnlyForward))
    std::cout << "##########    3. The next non-debug line should read: >>> 3" << std::endl;
  else if ((!Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (!Circular))
    std::cout << "##########    3. The next non-debug line should read: >>> 1" << std::endl;
  else if ((Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (!Circular))
    std::cout << "##########    3. The next non-debug line should read: >>> 4" << std::endl;
  else if ((!Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (Circular))
    std::cout << "##########    3. The next non-debug line should read: >>> 2" << std::endl;
  else if ((Reversed) && ((Iteration == safe::ForwardThenBackward) || (Iteration == safe::ForwardSameThenBackward)) && (Circular))
    std::cout << "##########    3. The next non-debug line should read: >>> 3" << std::endl;
  else if ((!Reversed) && (Iteration == safe::EvenErased))
    std::cout << "##########    3. The next non-debug line should read: >>> 2" << std::endl;
  else if ((Reversed) && (Iteration == safe::EvenErased))
    std::cout << "##########    3. The next non-debug line should read: >>> 3" << std::endl;

  --iter1;

  s = ">>> ";
  if (iter1 == map.end())
    s += "*\n";
  else
    s += std::to_string(iter1->first) + "\n";
  std::cout << s;

  std::cout << "##########    Testing for infinite loop (which of course shouldn't happen):" << std::endl;
  map.clear();
  ++iter1;
  std::cout << "##########    Test complete.  Trying in the other direction." << std::endl;
  --iter1;
  safe::map<int, MyValue, Circular, Iteration> map2;
  auto iter3 = map2.end();

  std::cout << "##########    Testing increment on empty map:" << std::endl;
  
  ++iter3;

  std::cout << "##########    Testing decrement on empty map:" << std::endl;
  
  --iter3;
  
  std::cout << "##########    Test complete." << std::endl;
}

// M A I N //////////////////////////////////////////////////////////////////

int main(int, char**)
{
  std::cout << "Start." << std::endl;
  
  // 1. Some miscellaneous initialization tests
  DEBUG_SIMPLE;  
  safe::map<int, number::weak<int>> map0{	// Try it with a non-class second argument
    {1, 1},
    {2, 2} 
  };
  map0.emplace(3,3);
  map0.erase(2);
  std::cout << map0.find(3)->second << std::endl;

  DEBUG_SIMPLE;
  safe::map<int, MyValue> map{
    {1, MyValue(1)},
    {2, MyValue(2)} 
  };
  DEBUG_SIMPLE;
  safe::map<int, MyValue> map2{
    {3, MyValue(4)},
    {3, MyValue(4)}
  };
  DEBUG_SIMPLE;
  std::map<int, MyValue> map3{
    {3, MyValue(4)},
    {3, MyValue(4)}
  };
  DEBUG_SIMPLE;
  map.swap(map2);
  DEBUG_SIMPLE;
  map = map2;
  DEBUG_SIMPLE;
  map = {
    {5, MyValue(5)},
    {6, MyValue(6)} 
  };
  DEBUG_SIMPLE;
  // 2. Constructor tests.

  map2 = safe::map<int, MyValue>();
  map2 = safe::map<int, MyValue>(std::less<int>());
  map2 = safe::map<int, MyValue>(std::allocator<std::pair<int, MyValue>>()); 
  map2 = safe::map<int, MyValue>(std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map.begin(), map.end());
  map2 = safe::map<int, MyValue>(map3.begin(), map3.end());
  map2 = safe::map<int, MyValue>(map.begin(), map.end(), std::less<int>());
  map2 = safe::map<int, MyValue>(map3.begin(), map3.end(), std::less<int>());
  map2 = safe::map<int, MyValue>(map.begin(), map.end(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map3.begin(), map3.end(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map.begin(), map.end(), std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map3.begin(), map3.end(), std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map);
  map2 = safe::map<int, MyValue>(map3);
  map2 = safe::map<int, MyValue>(map, std::less<int>());
  map2 = safe::map<int, MyValue>(map3, std::less<int>());
  map2 = safe::map<int, MyValue>(map, std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map3, std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map, std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(map3, std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(std::map<int, MyValue>());
  map2 = safe::map<int, MyValue>(safe::map<int, MyValue>());
  map2 = safe::map<int, MyValue>(std::map<int, MyValue>(), std::less<int>());
  map2 = safe::map<int, MyValue>(safe::map<int, MyValue>(), std::less<int>());
  map2 = safe::map<int, MyValue>(std::map<int, MyValue>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(safe::map<int, MyValue>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(std::map<int, MyValue>(), std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>(safe::map<int, MyValue>(), std::less<int>(), std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>({{1, MyValue(1)}, {2, MyValue(2)}});
  map2 = safe::map<int, MyValue>({{1, MyValue(1)}, {2, MyValue(2)}}, std::less<int>());
  map2 = safe::map<int, MyValue>({{1, MyValue(1)}, {2, MyValue(2)}}, std::allocator<std::pair<int, MyValue>>());
  map2 = safe::map<int, MyValue>({{1, MyValue(1)}, {2, MyValue(2)}}, std::less<int>(), std::allocator<std::pair<int, MyValue>>());

  // 3. Destructor safety tests.
  {
    auto map4 = new safe::map<int, MyValue, false, safe::OnlyForward>();
    map4->emplace(0, MyValue(0));
    map4->emplace(1, MyValue(1));
    auto iter1 = map4->begin();
    auto iter2 = map4->rbegin();
    std::cout << "##########    The map's destructor should be called here: " << std::endl;
    delete map4;
    std::cout << "##########    Two iterators should expire here, thus expiring the map: " << std::endl;
  }
  
  // 4. Iteration accuracy tests (non-circular).
  iteration_test<false, safe::OnlyForward, false>();
  iteration_test<true, safe::OnlyForward, false>();
  iteration_test<false, safe::ForwardThenBackward, false>();
  iteration_test<true, safe::ForwardThenBackward, false>();
  iteration_test<false, safe::ForwardSameThenBackward, false>();
  iteration_test<true, safe::ForwardSameThenBackward, false>();
  iteration_test<false, safe::EvenErased, false>();
  iteration_test<true, safe::EvenErased, false>();
  iteration_test<false, safe::OnlyForward, true>();
  iteration_test<true, safe::OnlyForward, true>();
  iteration_test<false, safe::ForwardThenBackward, true>();
  iteration_test<true, safe::ForwardThenBackward, true>();
  iteration_test<false, safe::ForwardSameThenBackward, true>();
  iteration_test<true, safe::ForwardSameThenBackward, true>();
  iteration_test<false, safe::EvenErased, true>();
  iteration_test<true, safe::EvenErased, true>();

  // 5. Now, the big part: the threaded stress tests. 

  map.clear();

  std::cout << "Inserting." << std::endl;

  for (int i = 0; i < 1000; ++i)
  {
    DEBUG_SIMPLE;
    map.insert(std::make_pair(rand(), MyValue(rand())));
    DEBUG_SIMPLE;
  }
  
  std::cout << "Launch threads." << std::endl;

  auto t1 = std::thread([&map]() { while (true) seeker_thread(map); });
  auto t2 = std::thread([&map]() { while (true) seeker_changer_thread(map); });
  auto t3 = std::thread([&map]() { while (true) scanner_thread(map); });
  auto t4 = std::thread([&map]() { while (true) reverse_scanner_thread(map); });
  auto t5 = std::thread([&map]() { while (true) scan_changer_thread(map); });
  auto t6 = std::thread([&map]() { while (true) { sleep(1); std::cout << "*********** " << map.size() << std::endl; } });

  std::cout << "Threads launched." << std::endl;

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  t6.join();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

