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

#if !defined(OGS_DBI_INSIDE) && !defined(OGS_DBI_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_DBI_MONGO_PRIVATE_H
#define OGS_DBI_MONGO_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dbi-private.h"

extern ogs_dbi_t ogs_dbi_mongo_interface;

/* ims */
int ogs_dbi_mongo_msisdn_data(
        char *imsi_or_msisdn_bcd, ogs_msisdn_data_t *msisdn_data);
int ogs_dbi_mongo_ims_data(char *supi, ogs_ims_data_t *ims_data);

/* session */
int ogs_dbi_mongo_session_data(char *supi, ogs_s_nssai_t *s_nssai, char *dnn,
        ogs_session_data_t *session_data);

/* subscription */
int ogs_dbi_mongo_auth_info(char *supi, ogs_dbi_auth_info_t *auth_info);
int ogs_dbi_mongo_update_sqn(char *supi, uint64_t sqn);
int ogs_dbi_mongo_update_imeisv(char *supi, char *imeisv);
int ogs_dbi_mongo_increment_sqn(char *supi);
int ogs_dbi_mongo_subscription_data(char *supi,
        ogs_subscription_data_t *subscription_data);

#ifdef __cplusplus
}
#endif

#endif /* OGS_DBI_MONGO_PRIVATE_H */
