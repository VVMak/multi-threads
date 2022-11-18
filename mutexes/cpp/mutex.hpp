#pragma once

class Mutex {
 public:
  virtual void Lock() = 0;
  virtual void Unlock() = 0;
  virtual ~Mutex() {}
};
