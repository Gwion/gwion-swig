//%typemap(freearg) char*, const char* %{release(temp, shred);%}
