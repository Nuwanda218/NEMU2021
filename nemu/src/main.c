void init_monitor(int, char *[]);
void reg_test();
void restart();
void ui_mainloop();
void test_expr();  // [PA1 stage2 mandatory task 3]

int main(int argc, char *argv[]) {

	/* Initialize the monitor. */
	init_monitor(argc, argv);

	/* Test the implementation of the `CPU_state' structure. */
	reg_test();

	/* Initialize the virtual computer system. */
	restart();

	/* Receive commands from user. */
	ui_mainloop();
    // [PA1 stage2 mandatory task 3]
	// Run test cases for arithmetic expression lexical analysis
	test_expr();



	return 0;
}
