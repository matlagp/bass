# Node client

## Important

To flash the image, partition size has to be increased.

Edit file: `~/.platformio/packages/framework-espidf/components/partition_table/partitions_singleapp.csv`

With contents:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x6000,
phy_init, data, phy,     ,        0x1000,
factory,  app,  factory, ,        2M,
```
