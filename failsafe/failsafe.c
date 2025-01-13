/* SPDX-License-Identifier:\tGPL-2.0 */
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 */

#include <common.h>
#include <malloc.h>
#include <net/tcp.h>
#include <net/httpd.h>
#include <u-boot/md5.h>

#include "fs.h"

static u32 upload_data_id;
static const void *upload_data;
static size_t upload_size;
static int upgrade_success;

extern int write_firmware_failsafe(size_t data_addr, uint32_t data_size);

static int output_plain_file(struct httpd_response *response,
	const char *filename)
{
	const struct fs_desc *file;
	int ret = 0;

	file = fs_find_file(filename);

	response->status = HTTP_RESP_STD;

	if (file) {
		response->data = file->data;
		response->size = file->size;
	} else {
		response->data = "Error: file not found";
		response->size = strlen(response->data);
		ret = 1;
	}

	response->info.code = 200;
	response->info.connection_close = 1;
	response->info.content_type = "text/html";

	return ret;
}

static void index_handler(enum httpd_uri_handler_status status,
	struct httpd_request *request,
	struct httpd_response *response)
{
	if (status == HTTP_CB_NEW)
		output_plain_file(response, "index.html");
}

// Function to validate a MAC address (manual validation)
int validate_mac(const char *mac) {
    if (strlen(mac) != 17) {
        return 0; // MAC address length must be 17 characters
    }

    for (int i = 0; i < 17; i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':') {
                return 0; // Every third character must be a colon
            }
        } else {
            if (!isxdigit(mac[i])) {
                return 0; // All other characters must be hex digits
            }
        }
    }
    return 1; // MAC address is valid
}

// Handle MAC address submission from the UI
void handle_mac_update(const char *mac1, const char *mac2, const char *mac3) {
    if (validate_mac(mac1)) {
        printf("Updating MAC Address 1: %s\n", mac1);
        run_command("setenv ethaddr %s", mac1);
    } else {
        printf("Invalid MAC Address 1: %s\n", mac1);
    }

    if (mac2 && validate_mac(mac2)) {
        printf("Updating MAC Address 2: %s\n", mac2);
        run_command("setenv eth1addr %s", mac2);
    } else if (mac2) {
        printf("Invalid MAC Address 2: %s\n", mac2);
    }

    if (mac3 && validate_mac(mac3)) {
        printf("Updating MAC Address 3: %s\n", mac3);
        run_command("setenv eth2addr %s", mac3);
    } else if (mac3) {
        printf("Invalid MAC Address 3: %s\n", mac3);
    }

    // Save the environment to persist changes
    run_command("saveenv");
}

// Example hook to process HTTP request (pseudo-code, adapt as per the framework)
void process_request(const char *request_data) {
    char mac1[18], mac2[18], mac3[18];
    // Extract MAC addresses from request_data (pseudo-parsing logic)
    sscanf(request_data, "mac1=%17s&mac2=%17s&mac3=%17s", mac1, mac2, mac3);

    // Handle MAC update
    handle_mac_update(mac1, mac2, mac3);
}
