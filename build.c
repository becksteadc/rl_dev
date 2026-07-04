#define INCLUDEBUILD_IMPLEMENTATION
#include "build.h"
#include <stdio.h>

int main(void) {
	ib_context *ctx = ib_context_create();
	ib_project *project = ib_project_create(ctx, ".");
	ib_target *target = NULL;
	ib_status status = IB_OK;

	if (!ctx || !project) {
		fprintf(stderr, "IncludeBuild initialization failed.\n");
		ib_project_destroy(project);
		ib_context_destroy(ctx);
		return 1;
	}

	ib_context_set_verbose(ctx, true);
	target = ib_project_add_target(project, "ib_game", IB_TARGET_EXECUTABLE);
	if (!target) {
		fprintf(stderr, "%s\n", ib_context_last_message(ctx));
		ib_project_destroy(project);
		ib_context_destroy(ctx);
		return 1;
	}

	status = ib_target_set_entry(target, "src/game_layer/main.c");
	if (status == IB_OK) {
		status = ib_target_add_cflags(target, "-DBUILD_CURSES");
	}
	if (status == IB_OK) {
		status = ib_target_add_link_flags(target, "-lcurses");
	}
	if (status == IB_OK) {
		status = ib_project_scan_shared_dir(project, "src", true);
	}
	if (status == IB_OK) {
		status = ib_project_build(project, IB_MODE_DEBUG);
	}

	if (status != IB_OK) {
		fprintf(stderr, "%s\n", ib_context_last_message(ctx));
	} else {
		puts("Built main.");
	}

	ib_project_destroy(project);
	ib_context_destroy(ctx);
	return status == IB_OK ? 0 : 1;
}
