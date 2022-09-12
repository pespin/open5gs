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

/* This file will be used when build without mongodb support to support the old api */
#include "ogs-dbi.h"

int ogs_dbi_init(const char *db_uri)
{
    return -ENODEV;
}

int ogs_dbi_mongo_init(const char *db_uri)
{
    return -ENODEV;
}

int ogs_mongoc_init(const char *db_uri)
{
    return -ENODEV;
}

void ogs_mongoc_final(void)
{
}

ogs_mongoc_t *ogs_mongoc(void)
{
    return NULL;
}


