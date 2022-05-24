# THREAD SAFETY

This document describes the analysis of thread safety of the librpma library ... XXX

## Thread-safe API calls

The following API calls of the librpma library are thread-safe:
- XXX

## NOT thread-safe API calls

The following API calls of the librpma library are NOT thread-safe:
- XXX

## Relationship of libibverbs and librdmacm

XXX

## Not thread-safe scenarios

XXX

## Analysis of Valgrind suppressions

### Suppressions for libibverbs

```
{
   Verbs context, once it is opened, it is never closed (RXE variant).
   Memcheck:Leak
   match-leak-kinds: possible
   fun:calloc
   ...
   fun:_verbs_init_and_alloc_context
   fun:rxe_alloc_context
   fun:verbs_open_device
   ...
   fun:ucma_init_device
}
```

```
{
   Verbs device, once it is opened, it is never closed.
   Memcheck:Leak
   match-leak-kinds: possible
   fun:calloc
   ...
   fun:try_driver
   fun:try_drivers
   fun:try_all_drivers
   fun:ibverbs_get_device_list
   fun:ibv_get_device_list*
   fun:ucma_init
   ...
}
```

```
{
   Sysfs device, once it is allocated, is not freed.
   Memcheck:Leak
   match-leak-kinds: possible
   fun:calloc
   fun:find_sysfs_devs_nl_cb
   fun:nl_recvmsgs_report
   fun:nl_recvmsgs
   fun:rdmanl_get_devices
   fun:find_sysfs_devs_nl
   fun:ibverbs_get_device_list
   fun:ibv_get_device_list*
   fun:ucma_init
   ...
}
```

### Suppressions for the drd tool

```
{
   Race while reading the name of time zone ("GMT").
   drd:ConflictingAccess
   ...
   fun:__tzfile_compute
   fun:__tz_convert
   fun:__vsyslog_internal
   fun:syslog
   fun:rpma_log_default_function
   ...
}
```

```
{
   Race while reading the name of time zone ("GMT").
   drd:ConflictingAccess
   fun:strcmp
   fun:__tzfile_compute
   fun:__tz_convert
   fun:get_timestamp_prefix
   fun:rpma_log_default_function
   ...
}
```

```
{
   Race while reading the name of time zone ("GMT").
   drd:ConflictingAccess
   fun:strlen
   fun:__tzstring
   fun:__tzfile_compute
   fun:__tz_convert
   fun:get_timestamp_prefix
   fun:rpma_log_default_function
   ...
}
```

```
{
   Race while reading the name of time zone ("GMT").
   drd:ConflictingAccess
   fun:bcmp
   fun:__tzstring_len
   fun:__tzstring
   fun:__tzfile_compute
   fun:__tz_convert
   fun:get_timestamp_prefix
   fun:rpma_log_default_function
   ...
}
```

cma_dev_cnt was a global counter of elements populating the global array of
devices detected by librdmacm. It was used as an indicator of already done
initialization. It was checked before locking a mutex required to make any
changes to the global array and changing the counter itself as follows:

```c
static int cma_dev_cnt;
...
int ucma_init(void)
{
    if (cma_dev_cnt)
        return 0;

    pthread_mutex_lock(&mut);
    if (cma_dev_cnt) {
        pthread_mutex_unlock(&mut);
        return 0;
    }
...
    cma_dev_cnt = dev_cnt;
...
    pthread_mutex_unlock(&mut);
...
}
```

But having a race, in this case, should do no harm since the counter is also
checked after locking the mutex. So, even writing a new value to the counter
even if it will be torn and read partially by another thread it won't lead
to abnormal behaviour.

Note: This issue is no longer the case for rdma-core >= v30.0 since there
instead of a global array is a global list. But the logic seems to be
the same: a quick check (a comparison) followed by the mutex lock and
recheck of the condition so the final assessment should be the same.

```
{
   Conflicting store/load of size 4
   drd:ConflictingAccess
   fun:ucma_init
   ...
}
```

```
{
   Conflicting store of size 4
   drd:ConflictingAccess
   fun:ucma_set_af_ib_support
   fun:ucma_init
   fun:ucma_init
   fun:rdma_getaddrinfo
   fun:rpma_info_new
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Conflicting load of size 8
   drd:ConflictingAccess
   fun:rdma_create_event_channel
   fun:ucma_alloc_id
   fun:rdma_create_id2.part.0
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Conflicting load of size 4
   drd:ConflictingAccess
   fun:UnknownInlinedFun
   fun:ucma_insert_id
   fun:rdma_create_id2.part.0
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Conflicting load of size 1
   drd:ConflictingAccess
   fun:strlen
   fun:__vfprintf_internal
   fun:__vasprintf_internal
   fun:__asprintf_chk
   fun:asprintf
   fun:open_cdev
   fun:rdma_create_event_channel
   fun:ucma_alloc_id
   fun:rdma_create_id2.part.0
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Conflicting load of size 4
   drd:ConflictingAccess
   fun:rdma_bind_addr
   fun:rpma_info_bind_addr
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Conflicting load of size 4
   drd:ConflictingAccess
   fun:ucma_get_device
   fun:ucma_query_route
   fun:rdma_bind_addr
   fun:rpma_info_bind_addr
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Conflicting load of size 4
   drd:ConflictingAccess
   fun:ucma_get_device
   fun:ucma_query_addr
   fun:rdma_bind_addr2
   fun:rdma_bind_addr
   fun:rpma_info_bind_addr
   fun:rpma_utils_get_ibv_context
   ...
}
```

Occurs in traces of:
- rpma_utils_get_ibv_context
- rpma_ep_shutdown

```
{
   Conflicting load of size 8
   drd:ConflictingAccess
   fun:idm_clear
   fun:ucma_remove_id
   fun:ucma_free_id
   fun:rdma_destroy_id
   ...
}
```

Occurs in traces of:
- rpma_utils_get_ibv_context
- rpma_info_new
- rpma_ep_listen

```
{
   Conflicting store of size 8
   drd:ConflictingAccess
   fun:idm_set
   fun:ucma_insert_id
   fun:rdma_create_id2.part.0
   ...
}
```

```
{
   Conflicting store of size 4
   drd:ConflictingAccess
   fun:ibv_dontfork_range
   fun:ibv_reg_mr_iova*
   fun:__ibv_reg_mr
   fun:rpma_peer_mr_reg
   fun:rpma_mr_reg
   ...
}
```

```
{
   Conflicting store of size 4
   drd:ConflictingAccess
   fun:ibv_dofork_range
   fun:ibv_dereg_mr*
   fun:rpma_mr_dereg
   ...
}
```

Occurs in traces of:
- rpma_conn_req_new
- rpma_conn_req_from_id

```
{
   Conflicting load/store of size 8
   drd:ConflictingAccess
   ...
   fun:_execute_ioctl_fallback
   ...
}
```

### Suppressions for the helgrind tool

```
{
   Race while reading the name of time zone ("GMT").
   Helgrind:Race
   ...
   fun:__tzfile_compute
   fun:__tz_convert
   fun:__vsyslog_internal
   fun:syslog
   fun:rpma_log_default_function
   ...
}
```

```
{
   Race while reading the name of time zone ("GMT").
   Helgrind:Race
   fun:strcmp
   fun:__tzfile_compute
   fun:__tz_convert
   fun:get_timestamp_prefix
   fun:rpma_log_default_function
   ...
}
```

```
{
   Race while reading the name of time zone ("GMT").
   Helgrind:Race
   fun:bcmp
   fun:__tzstring_len
   fun:__tzstring
   fun:__tzfile_compute
   fun:__tz_convert
   fun:get_timestamp_prefix
   fun:rpma_log_default_function
   ...
}
```

```
{
   Race while reading the name of time zone ("GMT").
   Helgrind:Race
   fun:strlen
   fun:__tzstring
   fun:__tzfile_compute
   fun:__tz_convert
   fun:get_timestamp_prefix
   fun:rpma_log_default_function
   ...
}
```

```
{
   Possible data race during write of size 4
   Helgrind:Race
   fun:ucma_init
   ...
}
```

```
{
   Possible data race during write of size 4
   Helgrind:Race
   fun:ucma_set_af_ib_support
   fun:ucma_init
   fun:ucma_init
   fun:rdma_getaddrinfo
   fun:rpma_info_new
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Possible data race during read of size 8
   Helgrind:Race
   fun:rdma_create_event_channel
   fun:ucma_alloc_id
   fun:rdma_create_id2.part.0
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Possible data race during read of size 4
   Helgrind:Race
   fun:UnknownInlinedFun
   fun:ucma_insert_id
   fun:rdma_create_id2.part.0
   fun:rpma_utils_get_ibv_context
   ...
}
```

Occurs in traces of:
- rpma_utils_get_ibv_context
- rpma_ep_listen
- rpma_info_new (ucma_set_af_ib_support)

```
{
   Possible data race during read of size 8
   Helgrind:Race
   fun:idm_set
   fun:ucma_insert_id
   fun:rdma_create_id2.part.0
   ...
}
```

```
{
   Possible data race during read of size 1
   Helgrind:Race
   fun:strlen
   fun:__vfprintf_internal
   fun:__vasprintf_internal
   fun:__asprintf_chk
   fun:asprintf
   fun:open_cdev
   fun:rdma_create_event_channel
   fun:ucma_alloc_id
   fun:rdma_create_id2.part.0
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Possible data race during read of size 4
   Helgrind:Race
   fun:rdma_bind_addr
   fun:rpma_info_bind_addr
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Possible data race during read of size 4
   Helgrind:Race
   fun:ucma_get_device
   fun:ucma_query_route
   fun:rdma_bind_addr
   fun:rpma_info_bind_addr
   fun:rpma_utils_get_ibv_context
   ...
}
```

```
{
   Possible data race during read of size 4
   Helgrind:Race
   fun:ucma_get_device
   fun:ucma_query_addr
   fun:rdma_bind_addr2
   fun:rdma_bind_addr
   fun:rpma_info_bind_addr
   fun:rpma_utils_get_ibv_context
   ...
}
```

Occurs in traces of:
- rpma_utils_get_ibv_context
- rpma_ep_shutdown
- rpma_info_new (ucma_set_af_ib_support)

```
{
   Possible data race during read of size 8
   Helgrind:Race
   fun:idm_clear
   fun:ucma_remove_id
   fun:ucma_free_id
   fun:rdma_destroy_id
   ...
}
```

```
{
   Possible data race during read of size 4
   Helgrind:Race
   fun:ibv_dontfork_range
   fun:ibv_reg_mr_iova*
   fun:__ibv_reg_mr
   fun:rpma_peer_mr_reg
   fun:rpma_mr_reg
   ...
}
```

```
{
   Possible data race during read of size 4
   Helgrind:Race
   fun:ibv_dofork_range
   fun:ibv_dereg_mr*
   fun:rpma_mr_dereg
   ...
}
```

Occurs in traces of:
- rpma_conn_req_new
- rpma_conn_req_from_id

```
{
   Possible data race during read/write of size 8
   Helgrind:Race
   ...
   fun:_execute_ioctl_fallback
   ...
}
```
