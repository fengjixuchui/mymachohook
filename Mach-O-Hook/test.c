#include <stdio.h>
#include <dlfcn.h>

#include "mach_hook.h"

void libtest();  //来自 libtest.dylib

int hooked_puts(char const *s)
{
    puts(s);  //　保持不变调用

    return puts("hook!");
}

int main()
{
    void *handle = 0;  //用于保存的句柄
    mach_substitution original;  //用于保存原始数据
	Dl_info info;

	if (!dladdr((void const *)libtest, &info))  //dladdrhttp://www.blogfshare.com/ioss-validate-address.html可获得一个函数所在模块，名称以及地址
    {
        fprintf(stderr, "Failed to get the base address of a library at `%s`!\n", info.dli_fname);

        goto end;
    }

    handle = mach_hook_init(info.dli_fname, info.dli_fbase);

    if (!handle)
    {
        fprintf(stderr, "Redirection init failed!\n");

        goto end;
    }

    libtest();  //calls puts() from libSystem.B.dylib

    puts("-----------------------------");

    original = mach_hook(handle, "puts", (mach_substitution)hooked_puts);

    if (!original)
    {
        fprintf(stderr, "Redirection failed!\n");

        goto end;
    }

    libtest();  //calls hooked_puts()

    puts("-----------------------------");

    original = mach_hook(handle, "puts", original);  //restores the original relocation

    if (!original)
    {
        fprintf(stderr, "Restoration failed!\n");

        goto end;
    }

    libtest();  //again calls puts() from libSystem.B.dylib

end:

    mach_hook_free(handle);
    handle = 0;  //no effect here, but just a good advice to prevent double freeing

    return 0;
}
