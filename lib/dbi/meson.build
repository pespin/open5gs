# Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

libdbi_conf = configuration_data()
libdbi_deps = [libcrypt_dep]

dbi_sources = files('''
    ogs-dbi.h
    dbi.c
'''.split())
libdbi_sources = [dbi_sources]

mongo_sources = files('''
    mongo/ogs-mongoc.h
    mongo/ogs-mongoc.c
    mongo/subscription.c
    mongo/session.c
    mongo/ims.c
'''.split())

without_mongo_sources = files('''
    mongo/dummy.c
    mongo/ogs-mongoc.h
'''.split())

json_sources = files('''
    json/json.c
    external/cjson/cJSON.c
'''.split())
libdbi_sources += [json_sources]
cjson_inc = include_directories('./external/')

mongodb_optval = get_option('mongodb')
if mongodb_optval
    libmongoc_dep = dependency('libmongoc-1.0', required: true)
    if libmongoc_dep.found()
        libdbi_conf.set('WITH_MONGOC', 1)
        libdbi_sources += [mongo_sources]
        libdbi_deps += [libmongoc_dep]
    endif
else
    libdbi_sources += [without_mongo_sources]
endif

configure_file(output : 'dbi-config-private.h', configuration : libdbi_conf)

libdbi_inc = include_directories('.')

dbi_cc_flags = ['-DOGS_DBI_COMPILATION']
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
    dbi_cc_flags += cc.get_supported_arguments([
                    '-Wno-float-equal',
    ])
endif

libdbi = library('ogsdbi',
    sources : libdbi_sources,
    version : libogslib_version,
    c_args : dbi_cc_flags,
    include_directories : [libdbi_inc, libinc, cjson_inc],
    dependencies : libdbi_deps,
    install : true)

libdbi_dep = declare_dependency(
    link_with : libdbi,
    include_directories : [libdbi_inc, libinc],
    dependencies : libdbi_deps)
