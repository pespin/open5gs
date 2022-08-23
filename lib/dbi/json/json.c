/*
 * Copyright (C) 2022 by sysmocom - s.f.m.c. GmbH <info@sysmocom.de>
 * Author: Alexander Couzens <lynxis@fe80.eu>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "ogs-dbi.h"
#include "json-private.h"

#include <cjson/cJSON.h>

#define MAX_DB_PROFILES 256
#define MAX_DB_APNS 10
typedef struct ogs_json_apn_profile_s {
    int32_t id;
    ogs_qos_t qos;
    bool valid;
} ogs_json_apn_profile_t;

typedef struct ogs_json_apn_s {
    char *apn;
    ogs_json_apn_profile_t profiles[MAX_DB_PROFILES];
    ogs_bitrate_t ambr; /* APN-AMBR */
    bool valid;
} ogs_json_apn_t;

ogs_json_apn_t g_apns[MAX_DB_APNS];

static ogs_json_apn_profile_t *ogs_dbi_static_alloc_db_entry(ogs_json_apn_t *db_apn)
{
    int i = 0;
    ogs_assert(db_apn);
    ogs_json_apn_profile_t *entry;

    for (i = 0; i < MAX_DB_PROFILES; i++) {
        entry = &db_apn->profiles[i];
        if (entry->valid)
            continue;

        return entry;
    }

    return NULL;
}

static ogs_json_apn_profile_t *ogs_dbi_static_get_apn_profile(ogs_json_apn_t *db_apn, int profile_id)
{
    int i = 0;
    ogs_assert(db_apn);

    for (i = 0; i < MAX_DB_PROFILES; i++) {
        ogs_json_apn_profile_t *entry = &db_apn->profiles[i];
        if (!entry->valid)
            continue;

        if (entry->id != profile_id)
            continue;

        return entry;
    }

    return NULL;
}

static ogs_json_apn_t *ogs_dbi_static_get_apn(const char *apn)
{
    int i;
    for (i = 0; i < MAX_DB_APNS; i++) {
        ogs_json_apn_t *entry = &g_apns[i];
        if (!entry->valid)
            continue;

        if (!ogs_strcasecmp(entry->apn, apn))
            return entry;
    }
    return NULL;
}


static ogs_json_apn_t *ogs_dbi_static_alloc_apn(void)
{
    int i = 0;
    ogs_json_apn_t *apn;

    for (i = 0; i < MAX_DB_APNS; i++) {
        apn = &g_apns[i];
        if (apn->valid)
            continue;

        return apn;
    }

    return NULL;
}

static int cjson_get_uint64_t(const cJSON * const top, const char *item_chr, int required, uint64_t min, uint64_t max, uint64_t *value)
{
    uint64_t number;

    ogs_assert(value);

    cJSON *item = cJSON_GetObjectItem(top, item_chr);
    if (!item) {
        if (!required)
            return 0;

        ogs_error("Could not find mandatory field %s\n", item_chr);
        return -ENOENT;
    }

    if (!cJSON_IsNumber(item)) {
        ogs_error("Invalid value for %s. It must be a number!\n", item_chr);
        return -EINVAL;
    }

    number = cJSON_GetNumberValue(item);
    if (!(min <= number && number <= max)) {
        ogs_error("Invalid value for %s: %ld. It must be %ld <= x <= %ld\n", item_chr, number, min, max);
        return -ERANGE;
    }

    *value = number;

    return 0;
}

static int cjson_get_int(const cJSON * const top, const char *item_chr, int required, int min, int max, int *value)
{
    int number;

    ogs_assert(value);

    cJSON *item = cJSON_GetObjectItem(top, item_chr);
    if (!item) {
        if (!required)
            return 0;

        ogs_error("Could not find mandatory field %s\n", item_chr);
        return -ENOENT;
    }

    if (!cJSON_IsNumber(item)) {
        ogs_error("Invalid value for %s. It must be a number!\n", item_chr);
        return -EINVAL;
    }

    number = cJSON_GetNumberValue(item);
    if (!(min <= number && number <= max)) {
        ogs_error("Invalid value for %s: %d. It must be %d <= x <= %d\n", item_chr, number, min, max);
        return -ERANGE;
    }

    *value = number;

    return 0;
}

static int get_bandwidth(cJSON *top, const char *mbr, ogs_bitrate_t *bitrate)
{
    int ret;
    ogs_assert(mbr);
    ogs_assert(top);
    ogs_assert(bitrate);

    cJSON *obj = cJSON_GetObjectItem(top, mbr);
    if (!obj)
        return 0;

    if (!cJSON_IsObject(obj)) {
        ogs_error("%s has the wrong type. It is not an object!\n", mbr);
        return -EINVAL;
    }

    /* TODO: this will break close to 2 gbit */
    ret = cjson_get_uint64_t(obj, "up", 0, 0, UINT64_MAX, &bitrate->uplink);
    if (ret)
        return ret;

    ret = cjson_get_uint64_t(obj, "down", 0, 0, UINT32_MAX, &bitrate->downlink);
    if (ret)
        return ret;

    return 0;
}

int ogs_dbi_json_init(const char *filepath, const char *apn)
{
    int ret = 0;
    long filesize = -1;
    ssize_t mem_read;
    ogs_json_apn_t *db_apn = ogs_dbi_static_get_apn(apn);
    FILE *filefp;
    char *buf = NULL;
    int profile_id;

    ogs_dbi_select_interface("json");

    if (db_apn) {
        return -EALREADY;
    }

    filefp = fopen(filepath, "r");
    if (!filefp) {
        return -ENOENT;
    }

    if (fseek(filefp, 0L, SEEK_END) < 0) {
        ret = -ENOENT;
        goto fail;
    }

    filesize = ftell(filefp);
    if (filesize < 0) {
        ret = -ENOENT;
        goto fail;
    }

    buf = ogs_malloc(filesize);
    if (!buf) {
        ret = -ENOMEM;
        goto fail;
    }

    if (fseek(filefp, 0L, SEEK_SET) < 0) {
        ret = -ENOENT;
        goto fail;
    }

    mem_read = fread(buf, filesize, 1, filefp);
    if (mem_read != 1 && !feof(filefp)) {
        ret = -1;
        goto fail;
    }

    cJSON *profiles = cJSON_ParseWithLength(buf, filesize);
    if (!profiles) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            ogs_error("Error before: %s\n", error_ptr);
        }

        ret = -1;
        goto fail;
    }

    if (!cJSON_IsArray(profiles)) {
        ogs_error("Profile root object must be an array");
        ret = -2;
        goto fail;
    }

    db_apn = ogs_dbi_static_alloc_apn();
    if (!db_apn) {
        ogs_error("Too many APNs\n");
        ret = -ENOENT;
        goto fail;
    }
    memset(db_apn, '\0', sizeof(*db_apn));
    db_apn->apn = ogs_strdup(apn);
    db_apn->valid = 1;

    cJSON *profile;
    cJSON_ArrayForEach(profile, profiles) {
        int number;
        ogs_json_apn_profile_t *entry;

        ret = cjson_get_int(profile, "id", 1, 0, 65535, &profile_id);
        if (ret)
            goto fail;

        entry = ogs_dbi_static_get_apn_profile(db_apn, profile_id);
        if (entry) {
            ogs_error("Doublicated profile with id %d\n", profile_id);
            ret = -1;
            goto fail;
        }

        entry = ogs_dbi_static_alloc_db_entry(db_apn);
        if (!entry) {
            ogs_error("Tooo many db entries for apn %s\n", apn);
            ret = -1;
            goto fail;
        }

        ogs_info("Loading json profile for apn: \"%s\" / charging id \"%d\"", apn, profile_id);
        entry->id = profile_id;

        ret = cjson_get_int(profile, "qci", 1, 0, 16, &number);
        if (ret)
            goto fail;

        entry->qos.index = number & 0xff;
        ret = get_bandwidth(profile, "ambr", &entry->qos.mbr);
        if (ret)
            goto fail;

        ret = get_bandwidth(profile, "gbr", &entry->qos.gbr);
        if (ret)
            goto fail;

        /* TODO: check boundary check for fields */
        ret = cjson_get_int(profile, "priority", 1, 0, 15, &number);
        if (ret)
            goto fail;
        entry->qos.arp.priority_level = number & 0xff;

        ret = cjson_get_int(profile, "pre_emption_capability", 1, 0, 1, &number);
        if (ret)
            goto fail;
        if (number)
            entry->qos.arp.pre_emption_capability = OGS_5GC_PRE_EMPTION_ENABLED;
        else
            entry->qos.arp.pre_emption_capability = OGS_5GC_PRE_EMPTION_DISABLED;

        ret = cjson_get_int(profile, "pre_emption_vulnerability", 1, 0, 1, &number);
        if (ret)
            goto fail;
        if (number)
            entry->qos.arp.pre_emption_vulnerability = OGS_5GC_PRE_EMPTION_ENABLED;
        else
            entry->qos.arp.pre_emption_vulnerability = OGS_5GC_PRE_EMPTION_DISABLED;
        entry->valid = true;
    }

    if (profiles)
        cJSON_Delete(profiles);

    if (buf)
        ogs_free(buf);

    return 0;

fail:
    if (db_apn->apn)
        ogs_free(db_apn->apn);

    if (profiles)
        cJSON_Delete(profiles);

    if (buf)
        ogs_free(buf);

    fclose(filefp);
    return ret;
}

static int json_session_data(char *supi, ogs_s_nssai_t *s_nssai, char *dnn,
        int32_t charging_char, ogs_session_data_t *session_data)
{
    ogs_session_t *session = &session_data->session;
    ogs_json_apn_profile_t *profile;
    ogs_json_apn_t *db_apn = ogs_dbi_static_get_apn(dnn);

    if (!db_apn)
        db_apn = ogs_dbi_static_get_apn("*");

    if (!db_apn) {
        ogs_error("Couldn't find a profile for dnn (%s)", dnn);
        return OGS_ERROR;
    }

    profile = ogs_dbi_static_get_apn_profile(db_apn, charging_char);
    /* try default profile */
    if (!profile && charging_char != -1)
        profile = ogs_dbi_static_get_apn_profile(db_apn, -1);

    if (!profile) {
        ogs_error("Couldn't find a profile for dnn (%s) with charging characteristic %d", dnn, charging_char);
        return OGS_ERROR;
    }

    session->name = ogs_strndup("json_profile", strlen("json_profile"));
    /* FIXME: the ambr shouldn't be qos.mbr, instead it should be from apn.ambr, but neither PCRF nor PCEF support it this way */
    session->ambr = profile->qos.mbr;
    session->qos = profile->qos;

    return OGS_OK;
}

static void json_final(void) {
    int i;
    for (i = 0; i < MAX_DB_APNS; i++) {
        ogs_json_apn_t *db_apn = &g_apns[i];
        if (!db_apn->valid)
            continue;
        db_apn->valid = 0;

        if (db_apn->apn) {
            ogs_free(db_apn->apn);
            db_apn->apn = NULL;
        }
    }
}

ogs_dbi_t ogs_dbi_json_interface = {
    .name = "json",
    /* session */
    .session_data = json_session_data,
    .final = json_final,
};

