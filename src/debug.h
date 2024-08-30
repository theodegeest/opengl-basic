#ifndef DEBUG_H_
#define DEBUG_H_

void glClearError();

void glLogError(const char *function_call, const char *file, int line);

#define GLCall(x)                                                              \
  glClearError();                                                              \
  x;                                                                           \
  glLogError(#x, __FILE__, __LINE__);

#endif // !DEBUG_H_
