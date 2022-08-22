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

#include "dbi-private.h"
#include "dbi-config-private.h"

#ifdef WITH_MONGOC
#include "mongo/mongo-private.h"
#endif

int __ogs_dbi_domain;

static ogs_dbi_t *dbi_interfaces[] = {
#ifdef WITH_MONGOC
    &ogs_dbi_mongo_interface,
#endif
    NULL,
};

static ogs_dbi_t *dbi_selected;

int ogs_dbi_deselect_interface()
{
    dbi_selected = NULL;

    return 0;
}

int ogs_dbi_select_interface(const char *dbi_name)
{
    ogs_dbi_t *dbi;
    int i;
    ogs_assert(dbi_name);

    for (i = 0; i < OGS_ARRAY_SIZE(dbi_interfaces); i++) {
        dbi = dbi_interfaces[i];
        if (!ogs_strcasecmp(dbi->name, dbi_name)) {
            dbi_selected = dbi;
            return 0;
        }
    }

    ogs_error("Couldn't find dbi interface %s", dbi_name);
    return -1;
}

/* ims */
int ogs_dbi_msisdn_data(
        char *imsi_or_msisdn_bcd, ogs_msisdn_data_t *msisdn_data)
{
    ogs_assert(dbi_selected);
    return dbi_selected->msisdn_data(imsi_or_msisdn_bcd, msisdn_data);
}

int ogs_dbi_ims_data(char *supi, ogs_ims_data_t *ims_data)
{
    ogs_assert(dbi_selected);
    return dbi_selected->ims_data(supi, ims_data);
}

/* session */
int ogs_dbi_session_data(char *supi, ogs_s_nssai_t *s_nssai, char *dnn,
        ogs_session_data_t *session_data)
{
    ogs_assert(dbi_selected);
    return dbi_selected->session_data(supi, s_nssai, dnn, session_data);
}

/* subscription */
int ogs_dbi_auth_info(char *supi, ogs_dbi_auth_info_t *auth_info)
{
    ogs_assert(dbi_selected);
    return dbi_selected->auth_info(supi, auth_info);
}

int ogs_dbi_update_sqn(char *supi, uint64_t sqn)
{
    ogs_assert(dbi_selected);
    return dbi_selected->update_sqn(supi, sqn);
}

int ogs_dbi_increment_sqn(char *supi)
{
    ogs_assert(dbi_selected);
    return dbi_selected->increment_sqn(supi);
}

int ogs_dbi_update_imeisv(char *supi, char *imeisv)
{
    ogs_assert(dbi_selected);
    return dbi_selected->update_imeisv(supi, imeisv);
}

int ogs_dbi_subscription_data(char *supi,
        ogs_subscription_data_t *subscription_data)
{
    ogs_assert(dbi_selected);
    return dbi_selected->subscription_data(supi, subscription_data);
}

