#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <edify/expr.h>
#include <common.h>
#include <cutils/properties.h>
#include <sys/mman.h>

#include <memory>
#include <string>
#include <vector>

extern "C" {
int flash_image(void *data, unsigned sz, const char *name);

/* Compatibility for Intel's GCC-built 32-bit static updater libraries. */
void __stack_chk_fail(void) __attribute__((noreturn));
void __stack_chk_fail_local(void)
{
	__stack_chk_fail();
}
}

Value* FlashOSImage(const char* name, State* state,
                    const std::vector<std::unique_ptr<Expr>>& argv)
{
	if (argv.size() != 2) {
		return ErrorAbort(state, "%s() expected 2 args, got %zu",
				  name, argv.size());
	}

	std::vector<std::unique_ptr<Value>> args;
	if (!ReadValueArgs(state, argv, &args))
		return nullptr;

	if (args[1]->type != VAL_STRING)
		return ErrorAbort(state, "partition argument to %s must be string",
				  name);

	const std::string& partition = args[1]->data;
	if (partition.empty())
		return ErrorAbort(state, "partition argument to %s can't be empty",
				  name);
	if (args[0]->data.empty())
		return ErrorAbort(state, "image argument to %s can't be empty", name);

	const size_t slash = partition.find_last_of('/');
	const std::string image_type = partition.substr(
			slash == std::string::npos ? 0 : slash + 1);
	if (image_type.empty())
		return ErrorAbort(state, "invalid partition argument to %s", name);

	if (flash_image(&args[0]->data[0], args[0]->data.size(),
			image_type.c_str()) != 0) {
		return ErrorAbort(state, "%s: Failed to flash image %s: %s",
				  name, image_type.c_str(), strerror(errno));
	}

	return StringValue("t");
}

void Register_libosip_updater(void)
{
	RegisterFunction("write_osip_image", FlashOSImage);
}
