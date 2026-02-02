extern "C" void print_string(const char* str, int pos);

extern "C" void kernel_main(){
	print_string("Hello from C++", 0);
}
