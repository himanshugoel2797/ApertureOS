#Virtual Memory Map
| Address Range           |Use                                         |
|-------------------------|--------------------------------------------|
|0x00000000 -> 0x40000000 | Kernel Low Memory                          |
|0x40000000 -> 0xFFFFBFFF | User Memory                                |
|0xFFFFBFFF -> 0xFFFFFFFF | User Stack                                 |

#Physical Memory Map
|Address Range            |Use                       |
|-------------------------|--------------------------|
|0x00000000 -> 0x30000000 | Kernel Low Memory        |
|0x30000000 -> 0xFFFFFFFF | User Memory              |
|0xFFFFFFFF+              | Kernel Managed page heap |
