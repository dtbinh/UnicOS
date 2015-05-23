#include <stdint.h>

#if ! MACOS
#include <string.h>
#endif

#include <lib.h>
#include <moduleLoader.h>
#include <naiveConsole.h>
#include <video.h>


extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const shellCodeModuleAddress = (void*)0x400000;
static void * const shellDataModuleAddress = (void*)0x500000;

typedef int (*EntryPoint)();

void clearBSS(void * bssAddress, uint64_t bssSize) {
	memset(bssAddress, 0, bssSize);
}

void * getStackBase() {
	return (void*)(
	           (uint64_t)&endOfKernel
	           + PageSize * 8				//The size of the stack itself, 32KiB
	           - sizeof(uint64_t)			//Begin at the top of the stack
	       );
}

void * initializeKernelBinary() {

	clearBSS(&bss, &endOfKernel - &bss);

	video_initialize();

	video_clear_screen();

	video_write_line("[x64BareBones]");

	video_write_string("  text: 0x");
	video_write_hex((uint64_t)&text);
	video_write_nl();

	video_write_string("  rodata: 0x");
	video_write_hex((uint64_t)&rodata);
	video_write_nl();

	video_write_string("  data: 0x");
	video_write_hex((uint64_t)&data);
	video_write_nl();

	video_write_string("  bss: 0x");
	video_write_hex((uint64_t)&bss);
	video_write_nl();

	video_write_line("[Done]");
	video_write_nl();
	video_write_nl();

	return getStackBase();
}

void load_kernel_modules() {

	video_write_line("[Loading modules]");

	void * moduleAddresses[] = {
		shellCodeModuleAddress,
		shellDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	video_write_line("[Done]");
	video_write_nl();

}

int main() {

	//load_kernel_modules();

	return 0;

	/*	video_initialize();

		video_clear_screen();


		for (int i = 1; i <= 29; ++i) {
			video_write_string("Imprimiendo linea: ");
			video_write_string(itoa(i, 10));
			video_write_nl();

		}

		video_write_nl();
		video_write_line("Linea nueva");



		return 0;*/

	ncPrint("[Kernel Main]");
	ncNewline();
	ncPrint("  Shell code module at 0x");
	ncPrintHex((uint64_t)shellCodeModuleAddress);
	ncNewline();
	ncPrint("  Calling the shell code module returned: ");
	ncPrintHex(((EntryPoint)shellCodeModuleAddress)());
	ncNewline();
	ncNewline();

	ncPrint("  Shell data module at 0x");
	ncPrintHex((uint64_t)shellDataModuleAddress);
	ncNewline();
	ncPrint("  Shell data module contents: ");
	ncPrint((char*)shellDataModuleAddress);
	ncNewline();

	//clear_screen();

	ncPrint("[Finished]");
	return 0;
}
