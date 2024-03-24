#pragma once
#ifndef Channel_H
#define Channel_H

class Program;

struct Channel {
  const Program *program;
  float volume;
};

#endif
