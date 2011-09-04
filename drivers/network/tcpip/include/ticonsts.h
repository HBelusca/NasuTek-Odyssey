/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TCP/IP protocol driver
 * FILE:        include/ticonsts.h
 * PURPOSE:     TCP/IP protocol driver constants
 */

#pragma once

/* NDIS version this driver supports */
#define NDIS_VERSION_MAJOR 4
#define NDIS_VERSION_MINOR 0

#ifdef _NTTEST_
/* Name of devices */
#define DD_TCP_DEVICE_NAME      L"\\Device\\NTTcp"
#define DD_UDP_DEVICE_NAME      L"\\Device\\NTUdp"
#define DD_IP_DEVICE_NAME       L"\\Device\\NTIp"
#define DD_RAWIP_DEVICE_NAME    L"\\Device\\NTRawIp"

/* For NDIS protocol registration */
#define IP_DEVICE_NAME          L"\\Device\\NTIp"
#else
#define DD_TCP_DEVICE_NAME      L"\\Device\\Tcp"
#define DD_UDP_DEVICE_NAME      L"\\Device\\Udp"
#define DD_IP_DEVICE_NAME       L"\\Device\\Ip"
#define DD_RAWIP_DEVICE_NAME    L"\\Device\\RawIp"

/* For NDIS protocol registration */
/* The DDK says you have to register with the name that's regstered with SCM, e.g. tcpip */
#define IP_DEVICE_NAME          L"\\Device\\Ip"
#define TCPIP_PROTOCOL_NAME     L"Tcpip"
#endif /* _NTTEST_ */

/* TCP/UDP/RawIP IOCTL code definitions */

#define FSCTL_TCP_BASE     FILE_DEVICE_NETWORK

#define _TCP_CTL_CODE(Function, Method, Access) \
    CTL_CODE(FSCTL_TCP_BASE, Function, Method, Access)

#define IOCTL_TCP_QUERY_INFORMATION_EX \
    _TCP_CTL_CODE(0, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_TCP_SET_INFORMATION_EX \
    _TCP_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_SET_IP_ADDRESS \
    _TCP_CTL_CODE(14, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_DELETE_IP_ADDRESS \
    _TCP_CTL_CODE(16, METHOD_BUFFERED, FILE_WRITE_ACCESS)

/* Unique error values for log entries */
#define TI_ERROR_DRIVERENTRY 0

/* Internal status codes */
#define IP_SUCCESS                 0x0000 /* Successful */
#define IP_NO_RESOURCES            0x0001 /* Not enough free resources */
#define IP_NO_ROUTE_TO_DESTINATION 0x0002 /* No route to destination */

/* EOF */
