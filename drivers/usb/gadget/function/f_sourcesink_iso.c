/*
 * f_sourcesink_iso.c - USB peripheral source/sink configuration driver ,for iso test
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* #define VERBOSE_DEBUG */
#define INT_TEST

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>

#include "g_zero.h"
#include <linux/usb/gadget_chips.h>
#include "../u_f.h"

struct f_sourcesink {
	struct usb_function function;

	struct usb_ep *in_ep;
	struct usb_ep *out_ep;
	struct usb_ep *int_in_ep;
};

static inline struct f_sourcesink *func_to_ss(struct usb_function *f)
{
	return container_of(f, struct f_sourcesink, function);
}

static unsigned pattern;
module_param(pattern, uint, 0644);
MODULE_PARM_DESC(pattern, "0 = all zeroes, 1 = mod63 ");

/*-------------------------------------------------------------------------*/

static struct usb_interface_descriptor source_sink_intf = {
	.bLength = sizeof source_sink_intf,
	.bDescriptorType = USB_DT_INTERFACE,

	.bNumEndpoints = 3,
	.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
	//.bAlternateSetting  = 5,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor fs_source_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = USB_DIR_IN,
	#ifdef INT_TEST
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	#else
	.bmAttributes = USB_ENDPOINT_XFER_ISOC,
	#endif
	//.wMaxPacketSize =     cpu_to_le16(256),
	.bInterval = 1,
};

static struct usb_endpoint_descriptor fs_sink_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_ISOC,
	//.wMaxPacketSize =     cpu_to_le16(256),
	.bInterval = 1,
};

static struct usb_endpoint_descriptor fs_int_source_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = cpu_to_le16(32),
	.bInterval = 4,
};

static struct usb_descriptor_header *fs_source_sink_descs[] = {
	(struct usb_descriptor_header *)&source_sink_intf,
	(struct usb_descriptor_header *)&fs_sink_desc,
	(struct usb_descriptor_header *)&fs_source_desc,
	(struct usb_descriptor_header *)&fs_int_source_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor hs_source_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	#ifdef INT_TEST
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	#else
	.bmAttributes = USB_ENDPOINT_XFER_ISOC,
	#endif
	.wMaxPacketSize = cpu_to_le16(256),
	.bInterval = 1,
};

static struct usb_endpoint_descriptor hs_sink_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bmAttributes = USB_ENDPOINT_XFER_ISOC,
	.wMaxPacketSize = cpu_to_le16(1024),
	.bInterval = 1,
};

static struct usb_endpoint_descriptor hs_int_source_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = cpu_to_le16(32),
	.bInterval = 8,
};

static struct usb_descriptor_header *hs_source_sink_descs[] = {
	(struct usb_descriptor_header *)&source_sink_intf,
	(struct usb_descriptor_header *)&hs_source_desc,
	(struct usb_descriptor_header *)&hs_sink_desc,
	(struct usb_descriptor_header *)&fs_int_source_desc,
	NULL,
};

/* super speed support: */

static struct usb_endpoint_descriptor ss_source_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bmAttributes = USB_ENDPOINT_XFER_ISOC,
	//.wMaxPacketSize =     cpu_to_le16(1024),
	.bInterval = 4,
};

struct usb_ss_ep_comp_descriptor ss_source_comp_desc = {
	.bLength = USB_DT_SS_EP_COMP_SIZE,
	.bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
	.bMaxBurst = 0,
	.bmAttributes = 0,
	.wBytesPerInterval = 0,
};

static struct usb_endpoint_descriptor ss_sink_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,

	.bmAttributes = USB_ENDPOINT_XFER_ISOC,
	//.wMaxPacketSize =     cpu_to_le16(1024),
	.bInterval = 1,
};

struct usb_ss_ep_comp_descriptor ss_sink_comp_desc = {
	.bLength = USB_DT_SS_EP_COMP_SIZE,
	.bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
	.bMaxBurst = 0,
	.bmAttributes = 0,
	.wBytesPerInterval = 0,
};

static struct usb_descriptor_header *ss_source_sink_descs[] = {
	(struct usb_descriptor_header *)&source_sink_intf,
	(struct usb_descriptor_header *)&ss_source_desc,
	(struct usb_descriptor_header *)&ss_source_comp_desc,
	(struct usb_descriptor_header *)&ss_sink_desc,
	(struct usb_descriptor_header *)&ss_sink_comp_desc,
	NULL,
};

/* function-specific strings: */

static struct usb_string strings_sourcesink[] = {
	[0].s = "source and sink data",
	{}			/* end of list */
};

static struct usb_gadget_strings stringtab_sourcesink = {
	.language = 0x0409,	/* en-us */
	.strings = strings_sourcesink,
};

static struct usb_gadget_strings *sourcesink_strings[] = {
	&stringtab_sourcesink,
	NULL,
};

/*-------------------------------------------------------------------------*/

static int __init
sourcesink_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_sourcesink *ss = func_to_ss(f);
	int id;

	/* allocate interface ID(s) */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	source_sink_intf.bInterfaceNumber = id;	//4;

	/* allocate endpoints */
	ss->in_ep = usb_ep_autoconfig(cdev->gadget, &fs_source_desc);
	if (!ss->in_ep) {
autoconf_fail:
		printk(KERN_NOTICE "%s: can't autoconfigure on %s\n",
		      f->name, cdev->gadget->name);
		return -ENODEV;
	}
	ss->in_ep->driver_data = cdev;	/* claim */

	ss->out_ep = usb_ep_autoconfig(cdev->gadget, &fs_sink_desc);
	if (!ss->out_ep)
		goto autoconf_fail;
	ss->out_ep->driver_data = cdev;	/* claim */

	ss->int_in_ep = usb_ep_autoconfig(cdev->gadget, &fs_int_source_desc);
	if (!ss->int_in_ep)
		goto autoconf_fail;
	ss->int_in_ep->driver_data = cdev;	/* claim */

	/* support high speed hardware */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		hs_source_desc.bEndpointAddress =
		    fs_source_desc.bEndpointAddress;
		hs_sink_desc.bEndpointAddress = fs_sink_desc.bEndpointAddress;
		hs_int_source_desc.bEndpointAddress =
		    fs_int_source_desc.bEndpointAddress;
		f->hs_descriptors = hs_source_sink_descs;
	}

	/* support super speed hardware */
	if (gadget_is_superspeed(c->cdev->gadget)) {
		ss_source_desc.bEndpointAddress =
		    fs_source_desc.bEndpointAddress;
		ss_sink_desc.bEndpointAddress = fs_sink_desc.bEndpointAddress;
		f->ss_descriptors = ss_source_sink_descs;
	}

	printk(KERN_NOTICE "%s speed %s: IN/%s, OUT/%s\n",
	    (gadget_is_superspeed(c->cdev->gadget) ? "super" :
	     (gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full")),
	    f->name, ss->in_ep->name, ss->out_ep->name);
	return 0;
}

static void
sourcesink_unbind(struct usb_configuration *c, struct usb_function *f)
{
	kfree(func_to_ss(f));
}

/* optionally require specific source/sink data patterns  */
int check_pattern = 0;
static int check_read_data(struct f_sourcesink *ss, struct usb_request *req)
{
	unsigned i;
	u8 *buf = req->buf;

	for (i = check_pattern; i < req->actual + check_pattern; i++, buf++) {
		switch (pattern) {

			/* all-zeroes has no synchronization issues */
		case 0:
			if (*buf == 0)
				continue;
			break;

			/* "mod63" stays in sync with short-terminated transfers,
			 * OR otherwise when host and gadget agree on how large
			 * each usb transfer request should be.  Resync is done
			 * with set_interface or set_config.  (We *WANT* it to
			 * get quickly out of sync if controllers or their drivers
			 * stutter for any reason, including buffer duplcation...)
			 */
		case 1:
			if (*buf == (u8) (i % 63))
				continue;
			break;
		}
		printk(KERN_NOTICE "bad OUT byte, buf[%d] = %d\n", i, *buf);
		usb_ep_set_halt(ss->out_ep);
		check_pattern++;
		return -EINVAL;
	}
	return 0;
}

static void reinit_write_data(struct usb_ep *ep, struct usb_request *req)
{
	unsigned i;
	u8 *buf = req->buf;

	switch (pattern) {
	case 0:
		memset(req->buf, 0, req->length);
		break;
	case 1:
		for (i = 0; i < req->length; i++)
			*buf++ = (u8) (i % 63);
		break;
	}
}

static void source_sink_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_sourcesink *ss = ep->driver_data;
	int status = req->status;

	switch (status) {

	case 0:		/* normal completion? */
		if (ep == ss->out_ep) {
			check_read_data(ss, req);
			memset(req->buf, 0x55, req->length);
		} else
			reinit_write_data(ep, req);
		break;

		/* this endpoint is normally active while we're configured */
	case -ECONNABORTED:	/* hardware forced ep reset */
	case -ECONNRESET:	/* request dequeued */
	case -ESHUTDOWN:	/* disconnect from host */
		printk(KERN_NOTICE "%s gone (%d), %d/%d\n", ep->name, status,
		     req->actual, req->length);
		if (ep == ss->out_ep)
			check_read_data(ss, req);
		free_ep_req(ep, req);
		return;

	case -EOVERFLOW:	/* buffer overrun on read means that
				 * we didn't provide a big enough
				 * buffer.
				 */
	default:
#if 1
		printk(KERN_NOTICE "%s complete --> %d, %d/%d\n", ep->name,
		    status, req->actual, req->length);
#endif
	case -EREMOTEIO:	/* short read */
		break;
	}
	printk(KERN_NOTICE "****source_sink_complete &&&&&\n");
	status = usb_ep_queue(ep, req, GFP_ATOMIC);
	if (status) {
		printk(KERN_NOTICE "kill %s:  resubmit %d bytes --> %d\n",
		      ep->name, req->length, status);
		usb_ep_set_halt(ep);
		/* FIXME recover later ... somehow */
	}
}

static int source_sink_start_ep(struct f_sourcesink *ss, bool is_in)
{
	struct usb_ep *ep;
	struct usb_request *req;
	int status;

	ep = is_in ? ss->in_ep : ss->out_ep;
	req = alloc_ep_req(ep);
	if (!req)
		return -ENOMEM;

	req->complete = source_sink_complete;
	if (is_in)
		reinit_write_data(ep, req);
	else
		memset(req->buf, 0x55, req->length);
#if 1
	printk(KERN_NOTICE "is_in = %d ep addr %x\n", is_in, ep->address);
	status = usb_ep_queue(ep, req, GFP_ATOMIC);
	if (status) {
		struct usb_composite_dev *cdev;

		cdev = ss->function.config->cdev;
		printk(KERN_NOTICE "start %s %s --> %d\n",
		      is_in ? "IN" : "OUT", ep->name, status);
		free_ep_req(ep, req);
	}

	return status;
#else
	return 0;
#endif
}

static void disable_source_sink(struct f_sourcesink *ss)
{
	struct usb_composite_dev *cdev;

	cdev = ss->function.config->cdev;
	disable_endpoints(cdev, ss->in_ep, ss->out_ep);
	printk(KERN_NOTICE "%s disabled\n", ss->function.name);
}

static int
enable_source_sink(struct usb_composite_dev *cdev, struct f_sourcesink *ss)
{
	int result = 0;
	struct usb_ep *ep;

	/* one endpoint writes (sources) zeroes IN (to the host) */
	ep = ss->in_ep;
	result = config_ep_by_speed(cdev->gadget, &(ss->function), ep);
	if (result)
		return result;
	result = usb_ep_enable(ep);
	if (result < 0)
		return result;
	ep->driver_data = ss;

	result = source_sink_start_ep(ss, true);
	if (result < 0) {
fail:
		ep = ss->in_ep;
		usb_ep_disable(ep);
		ep->driver_data = NULL;
		return result;
	}

	/* one endpoint reads (sinks) anything OUT (from the host) */
	ep = ss->out_ep;
	result = config_ep_by_speed(cdev->gadget, &(ss->function), ep);
	if (result)
		goto fail;
	result = usb_ep_enable(ep);
	if (result < 0)
		goto fail;
	ep->driver_data = ss;

	result = source_sink_start_ep(ss, false);
	if (result < 0) {
		usb_ep_disable(ep);
		ep->driver_data = NULL;
		goto fail;
	}

	printk(KERN_NOTICE "%s enabled\n", ss->function.name);
	return result;
}

static int sourcesink_set_alt(struct usb_function *f,
			      unsigned intf, unsigned alt)
{
	struct f_sourcesink *ss = func_to_ss(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	/* we know alt is zero */
	if (ss->in_ep->driver_data)
		disable_source_sink(ss);
	return enable_source_sink(cdev, ss);
}

static void sourcesink_disable(struct usb_function *f)
{
	struct f_sourcesink *ss = func_to_ss(f);

	disable_source_sink(ss);
}

/*-------------------------------------------------------------------------*/

static int __init sourcesink_bind_config(struct usb_configuration *c)
{
	struct f_sourcesink *ss;
	int status;

	ss = kzalloc(sizeof *ss, GFP_KERNEL);
	if (!ss)
		return -ENOMEM;

	ss->function.name = "source/sink";
	ss->function.fs_descriptors = fs_source_sink_descs;
	ss->function.bind = sourcesink_bind;
	ss->function.unbind = sourcesink_unbind;
	ss->function.set_alt = sourcesink_set_alt;
	ss->function.disable = sourcesink_disable;

	status = usb_add_function(c, &ss->function);
	if (status)
		kfree(ss);
	return status;
}

static int sourcesink_setup(struct usb_configuration *c,
			    const struct usb_ctrlrequest *ctrl)
{
	struct usb_request *req = c->cdev->req;
	int value = -EOPNOTSUPP;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);

	req->length = USB_BUFSIZ;

	/* composite driver infrastructure handles everything except
	 * the two control test requests.
	 */
	switch (ctrl->bRequest) {

		/*
		 * These are the same vendor-specific requests supported by
		 * Intel's USB 2.0 compliance test devices.  We exceed that
		 * device spec by allowing multiple-packet requests.
		 *
		 * NOTE:  the Control-OUT data stays in req->buf ... better
		 * would be copying it into a scratch buffer, so that other
		 * requests may safely intervene.
		 */
	case 0x5b:		/* control WRITE test -- fill the buffer */
		if (ctrl->bRequestType != (USB_DIR_OUT | USB_TYPE_VENDOR))
			goto unknown;
		if (w_value || w_index)
			break;
		/* just read that many bytes into the buffer */
		if (w_length > req->length)
			break;
		value = w_length;
		break;
	case 0x5c:		/* control READ test -- return the buffer */
		if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_VENDOR))
			goto unknown;
		if (w_value || w_index)
			break;
		/* expect those bytes are still in the buffer; send back */
		if (w_length > req->length)
			break;
		value = w_length;
		break;

	default:
unknown:
		printk(KERN_NOTICE "unknown control req%02x.%02x v%04x i%04x l%d\n",
		     ctrl->bRequestType, ctrl->bRequest,
		     w_value, w_index, w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		printk(KERN_NOTICE "source/sink req%02x.%02x v%04x i%04x l%d\n",
		     ctrl->bRequestType, ctrl->bRequest,
		     w_value, w_index, w_length);
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(c->cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(c->cdev, "source/sinkc response, err %d\n",
			      value);
	}

	/* device either stalls (value < 0) or reports success */
	return value;
}

static struct usb_configuration sourcesink_driver = {
	.label = "source/sink",
	.strings = sourcesink_strings,
	.setup = sourcesink_setup,
	.bConfigurationValue = 1,
	.bmAttributes = USB_CONFIG_ATT_SELFPOWER,
	/* .iConfiguration = DYNAMIC */
};

/**
 * sourcesink_add - add a source/sink testing configuration to a device
 * @cdev: the device to support the configuration
 */
int __init sourcesink_add(struct usb_composite_dev *cdev, bool autoresume)
{
	int id;

	/* allocate string ID(s) */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_sourcesink[0].id = id;

	source_sink_intf.iInterface = id;
	sourcesink_driver.iConfiguration = id;

	/* support autoresume for remote wakeup testing */
	if (autoresume)
		sourcesink_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;

	/* support OTG systems */
	if (gadget_is_otg(cdev->gadget)) {
		sourcesink_driver.descriptors = otg_desc;
		sourcesink_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	return usb_add_config(cdev, &sourcesink_driver, sourcesink_bind_config);
}