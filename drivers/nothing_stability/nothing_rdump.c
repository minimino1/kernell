#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#define DRIVER_NAME "nt_rdump"

static void __iomem *imem_base;
static u64 lba_val;

/* Module parameter callbacks to read/write LBA value */
static int lba_param_set(const char *val, const struct kernel_param *kp)
{
    unsigned long long tmp;
    int ret = kstrtoull(val, 10, &tmp);
    if (ret)
        return ret;

    lba_val = tmp;
    if (imem_base) {
        writeq(tmp, imem_base);
    }
    return 0;
}

static int lba_param_get(char *buffer, const struct kernel_param *kp)
{
    u64 tmp = lba_val;
    if (imem_base)
        tmp = readq(imem_base);
    return sprintf(buffer, "%llu\n", tmp);
}

static const struct kernel_param_ops lba_param_ops = {
    .set = lba_param_set,
    .get = lba_param_get,
};
module_param_cb(lba_addr, &lba_param_ops, &lba_val, 0644);
MODULE_PARM_DESC(lba_addr, "LBA address stored in OCIMEM via DT reg");


static const struct of_device_id nothing_lba_of_match[] = {
    { .compatible = "nothing,lba_addr" },
    {},
};
MODULE_DEVICE_TABLE(of, nothing_lba_of_match);

static int nothing_lba_probe(struct platform_device *pdev)
{
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(&pdev->dev, "failed to get memory resource\n");
        return -ENODEV;
    }

    imem_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(imem_base)) {
        dev_err(&pdev->dev, "ioremap failed\n");
        return PTR_ERR(imem_base);
    }

    dev_info(&pdev->dev, "mapped IMEM at %pa size %llx\n", &res->start, resource_size(res));
    return 0;
}

static int nothing_lba_remove(struct platform_device *pdev)
{
    imem_base = NULL;
    return 0;
}

static struct platform_driver nothing_lba_driver = {
    .probe = nothing_lba_probe,
    .remove = nothing_lba_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = nothing_lba_of_match,
    },
};

module_platform_driver(nothing_lba_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("<BSP_CORE@nothing.tech>");
MODULE_DESCRIPTION("LBA Address IMEM driver");

