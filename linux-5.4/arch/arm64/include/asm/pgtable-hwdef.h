/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Ltd.
 */
#ifndef __ASM_PGTABLE_HWDEF_H
#define __ASM_PGTABLE_HWDEF_H

#include <asm/memory.h>

/*
 * Number of page-table levels required to address 'va_bits' wide
 * address, without section mapping. We resolve the top (va_bits - PAGE_SHIFT)
 * bits with (PAGE_SHIFT - 3) bits at each page table level. Hence:
 *
 *  levels = DIV_ROUND_UP((va_bits - PAGE_SHIFT), (PAGE_SHIFT - 3))
 *
 * where DIV_ROUND_UP(n, d) => (((n) + (d) - 1) / (d))
 *
 * We cannot include linux/kernel.h which defines DIV_ROUND_UP here
 * due to build issues. So we open code DIV_ROUND_UP here:
 *
 *	((((va_bits) - PAGE_SHIFT) + (PAGE_SHIFT - 3) - 1) / (PAGE_SHIFT - 3))
 *
 * which gets simplified as :
 */
#define ARM64_HW_PGTABLE_LEVELS(va_bits) (((va_bits) - 4) / (PAGE_SHIFT - 3))

/*
 * Size mapped by an entry at level n ( 0 <= n <= 3)
 * We map (PAGE_SHIFT - 3) at all translation levels and PAGE_SHIFT bits
 * in the final page. The maximum number of translation levels supported by
 * the architecture is 4. Hence, starting at at level n, we have further
 * ((4 - n) - 1) levels of translation excluding the offset within the page.
 * So, the total number of bits mapped by an entry at level n is :
 *
 *  ((4 - n) - 1) * (PAGE_SHIFT - 3) + PAGE_SHIFT
 *
 * Rearranging it a bit we get :
 *   (4 - n) * (PAGE_SHIFT - 3) + 3
 */
#define ARM64_HW_PGTABLE_LEVEL_SHIFT(n)	((PAGE_SHIFT - 3) * (4 - (n)) + 3)

//Linux内核默认页面粒度为4KB，则页表的偏移量是从第12位开始的
#define PTRS_PER_PTE		(1 << (PAGE_SHIFT - 3))

/*
 * PMD_SHIFT 决定二级页表项可以映射的大小
 */
#if CONFIG_PGTABLE_LEVELS > 2
// PMD_SHIFT宏 表示 PMD 页表在虚拟地址中的起始偏移量
#define PMD_SHIFT		ARM64_HW_PGTABLE_LEVEL_SHIFT(2)
// PMD_SIZE 宏表示 一个PMD 页表项所能映射的区域大小
#define PMD_SIZE		(_AC(1, UL) << PMD_SHIFT)
// PMD_MASK 宏用来屏蔽虚拟地址中 PT 索引字段的所有位 
#define PMD_MASK		(~(PMD_SIZE-1))
// PTRS_PER_PUD 宏表示 PMD 页表中页表项的个数
#define PTRS_PER_PMD		PTRS_PER_PTE
#endif

/*
 * PUD_SHIFT 决定了1级页表项可以映射的大小
 */
#if CONFIG_PGTABLE_LEVELS > 3
// PUD_SHIFT宏 表示 PUD 页表在虚拟地址中的起始偏移量
#define PUD_SHIFT		ARM64_HW_PGTABLE_LEVEL_SHIFT(1)
// PUD_SIZE 宏表示 一个PUD 页表项所能映射的区域大小
#define PUD_SIZE		(_AC(1, UL) << PUD_SHIFT)
// PUD_MASK 宏用来屏蔽虚拟地址中的 PMD 索引和 PT 索引字段的所有位 
#define PUD_MASK		(~(PUD_SIZE-1))
// PTRS_PER_PUD 宏表示 PUD 页表中页表项的个数
#define PTRS_PER_PUD		PTRS_PER_PTE
#endif

/*
 * PGDIR_SHIFT 决定了顶级页表项可以映射的大小
 * (depending on the configuration, this level can be 0, 1 or 2).
 */
// PGDIR_SHIFT宏 表示 PGD 页表在虚拟地址中的起始偏移量
#define PGDIR_SHIFT		ARM64_HW_PGTABLE_LEVEL_SHIFT(4 - CONFIG_PGTABLE_LEVELS)
// PGDIR_SIZE 宏表示PGD页表项所能映射的区域大小
#define PGDIR_SIZE		(_AC(1, UL) << PGDIR_SHIFT)
// PGDIR_MASK 宏用来屏蔽虚拟地址中的PUD 索引和 PMD 索引以及 PT 索引字段的所有位
#define PGDIR_MASK		(~(PGDIR_SIZE-1))
// PTRS_PER_PGD 宏表示 PGD 页表中页表项的个数
#define PTRS_PER_PGD		(1 << (MAX_USER_VA_BITS - PGDIR_SHIFT))

/*
 * Section address mask and size definitions.
 */
#define SECTION_SHIFT		PMD_SHIFT
#define SECTION_SIZE		(_AC(1, UL) << SECTION_SHIFT)
#define SECTION_MASK		(~(SECTION_SIZE-1))

/*
 * Contiguous page definitions.
 */
#ifdef CONFIG_ARM64_64K_PAGES
#define CONT_PTE_SHIFT		5
#define CONT_PMD_SHIFT		5
#elif defined(CONFIG_ARM64_16K_PAGES)
#define CONT_PTE_SHIFT		7
#define CONT_PMD_SHIFT		5
#else
#define CONT_PTE_SHIFT		4
#define CONT_PMD_SHIFT		4
#endif

#define CONT_PTES		(1 << CONT_PTE_SHIFT)
#define CONT_PTE_SIZE		(CONT_PTES * PAGE_SIZE)
#define CONT_PTE_MASK		(~(CONT_PTE_SIZE - 1))
#define CONT_PMDS		(1 << CONT_PMD_SHIFT)
#define CONT_PMD_SIZE		(CONT_PMDS * PMD_SIZE)
#define CONT_PMD_MASK		(~(CONT_PMD_SIZE - 1))
/* the the numerical offset of the PTE within a range of CONT_PTES */
#define CONT_RANGE_OFFSET(addr) (((addr)>>PAGE_SHIFT)&(CONT_PTES-1))

/*
 * Hardware page table definitions.
 *
  * L1页表项描述符（PUD）
 */
#define PUD_TYPE_TABLE		(_AT(pudval_t, 3) << 0)
#define PUD_TABLE_BIT		(_AT(pudval_t, 1) << 1)
#define PUD_TYPE_MASK		(_AT(pudval_t, 3) << 0)
#define PUD_TYPE_SECT		(_AT(pudval_t, 1) << 0)

/*
 * L2页表项描述符（PMD）
 */
#define PMD_TYPE_MASK		(_AT(pmdval_t, 3) << 0)
#define PMD_TYPE_TABLE		(_AT(pmdval_t, 3) << 0)
#define PMD_TYPE_SECT		(_AT(pmdval_t, 1) << 0)
#define PMD_TABLE_BIT		(_AT(pmdval_t, 1) << 1)

/*
 * Section
 */
#define PMD_SECT_VALID		(_AT(pmdval_t, 1) << 0)
#define PMD_SECT_USER		(_AT(pmdval_t, 1) << 6)		/* AP[1] */
#define PMD_SECT_RDONLY		(_AT(pmdval_t, 1) << 7)		/* AP[2] */
#define PMD_SECT_S		(_AT(pmdval_t, 3) << 8)
#define PMD_SECT_AF		(_AT(pmdval_t, 1) << 10)
#define PMD_SECT_NG		(_AT(pmdval_t, 1) << 11)
#define PMD_SECT_CONT		(_AT(pmdval_t, 1) << 52)
#define PMD_SECT_PXN		(_AT(pmdval_t, 1) << 53)
#define PMD_SECT_UXN		(_AT(pmdval_t, 1) << 54)

/*
 * AttrIndx[2:0] encoding (mapping attributes defined in the MAIR* registers).
 */
#define PMD_ATTRINDX(t)		(_AT(pmdval_t, (t)) << 2)
#define PMD_ATTRINDX_MASK	(_AT(pmdval_t, 7) << 2)

/*
 * L3页表项描述符（PTE）
 */
#define PTE_VALID		(_AT(pteval_t, 1) << 0)
#define PTE_TYPE_MASK		(_AT(pteval_t, 3) << 0)
#define PTE_TYPE_PAGE		(_AT(pteval_t, 3) << 0)
#define PTE_TABLE_BIT		(_AT(pteval_t, 1) << 1)
#define PTE_USER		(_AT(pteval_t, 1) << 6)		/* AP[1] */
#define PTE_RDONLY		(_AT(pteval_t, 1) << 7)		/* AP[2] */
#define PTE_SHARED		(_AT(pteval_t, 3) << 8)		/* SH[1:0], inner shareable */
#define PTE_AF			(_AT(pteval_t, 1) << 10)	/* Access Flag */
#define PTE_NG			(_AT(pteval_t, 1) << 11)	/* nG */
#define PTE_DBM			(_AT(pteval_t, 1) << 51)	/* Dirty Bit Management */
#define PTE_CONT		(_AT(pteval_t, 1) << 52)	/* Contiguous range */
#define PTE_PXN			(_AT(pteval_t, 1) << 53)	/* Privileged XN */
#define PTE_UXN			(_AT(pteval_t, 1) << 54)	/* User XN */
#define PTE_HYP_XN		(_AT(pteval_t, 1) << 54)	/* HYP XN */

#define PTE_ADDR_LOW		(((_AT(pteval_t, 1) << (48 - PAGE_SHIFT)) - 1) << PAGE_SHIFT)
#ifdef CONFIG_ARM64_PA_BITS_52
#define PTE_ADDR_HIGH		(_AT(pteval_t, 0xf) << 12)
#define PTE_ADDR_MASK		(PTE_ADDR_LOW | PTE_ADDR_HIGH)
#else
#define PTE_ADDR_MASK		PTE_ADDR_LOW
#endif

/*
 * AttrIndx[2:0] encoding (mapping attributes defined in the MAIR* registers).
 */
#define PTE_ATTRINDX(t)		(_AT(pteval_t, (t)) << 2)
#define PTE_ATTRINDX_MASK	(_AT(pteval_t, 7) << 2)

/*
 * 2nd stage PTE definitions
 */
#define PTE_S2_RDONLY		(_AT(pteval_t, 1) << 6)   /* HAP[2:1] */
#define PTE_S2_RDWR		(_AT(pteval_t, 3) << 6)   /* HAP[2:1] */
#define PTE_S2_XN		(_AT(pteval_t, 2) << 53)  /* XN[1:0] */

#define PMD_S2_RDONLY		(_AT(pmdval_t, 1) << 6)   /* HAP[2:1] */
#define PMD_S2_RDWR		(_AT(pmdval_t, 3) << 6)   /* HAP[2:1] */
#define PMD_S2_XN		(_AT(pmdval_t, 2) << 53)  /* XN[1:0] */

#define PUD_S2_RDONLY		(_AT(pudval_t, 1) << 6)   /* HAP[2:1] */
#define PUD_S2_RDWR		(_AT(pudval_t, 3) << 6)   /* HAP[2:1] */
#define PUD_S2_XN		(_AT(pudval_t, 2) << 53)  /* XN[1:0] */

/*
 * Memory Attribute override for Stage-2 (MemAttr[3:0])
 */
#define PTE_S2_MEMATTR(t)	(_AT(pteval_t, (t)) << 2)
#define PTE_S2_MEMATTR_MASK	(_AT(pteval_t, 0xf) << 2)

/*
 * EL2/HYP PTE/PMD definitions
 */
#define PMD_HYP			PMD_SECT_USER
#define PTE_HYP			PTE_USER

/*
 * Highest possible physical address supported.
 */
#define PHYS_MASK_SHIFT		(CONFIG_ARM64_PA_BITS)
#define PHYS_MASK		((UL(1) << PHYS_MASK_SHIFT) - 1)

#define TTBR_CNP_BIT		(UL(1) << 0)

/*
 * TCR flags.
 */
#define TCR_T0SZ_OFFSET		0
#define TCR_T1SZ_OFFSET		16
#define TCR_T0SZ(x)		((UL(64) - (x)) << TCR_T0SZ_OFFSET)
#define TCR_T1SZ(x)		((UL(64) - (x)) << TCR_T1SZ_OFFSET)
#define TCR_TxSZ(x)		(TCR_T0SZ(x) | TCR_T1SZ(x))
#define TCR_TxSZ_WIDTH		6
#define TCR_T0SZ_MASK		(((UL(1) << TCR_TxSZ_WIDTH) - 1) << TCR_T0SZ_OFFSET)

#define TCR_EPD0_SHIFT		7
#define TCR_EPD0_MASK		(UL(1) << TCR_EPD0_SHIFT)
#define TCR_IRGN0_SHIFT		8
#define TCR_IRGN0_MASK		(UL(3) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_NC		(UL(0) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WBWA		(UL(1) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WT		(UL(2) << TCR_IRGN0_SHIFT)
#define TCR_IRGN0_WBnWA		(UL(3) << TCR_IRGN0_SHIFT)

#define TCR_EPD1_SHIFT		23
#define TCR_EPD1_MASK		(UL(1) << TCR_EPD1_SHIFT)
#define TCR_IRGN1_SHIFT		24
#define TCR_IRGN1_MASK		(UL(3) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_NC		(UL(0) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WBWA		(UL(1) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WT		(UL(2) << TCR_IRGN1_SHIFT)
#define TCR_IRGN1_WBnWA		(UL(3) << TCR_IRGN1_SHIFT)

#define TCR_IRGN_NC		(TCR_IRGN0_NC | TCR_IRGN1_NC)
#define TCR_IRGN_WBWA		(TCR_IRGN0_WBWA | TCR_IRGN1_WBWA)
#define TCR_IRGN_WT		(TCR_IRGN0_WT | TCR_IRGN1_WT)
#define TCR_IRGN_WBnWA		(TCR_IRGN0_WBnWA | TCR_IRGN1_WBnWA)
#define TCR_IRGN_MASK		(TCR_IRGN0_MASK | TCR_IRGN1_MASK)


#define TCR_ORGN0_SHIFT		10
#define TCR_ORGN0_MASK		(UL(3) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_NC		(UL(0) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WBWA		(UL(1) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WT		(UL(2) << TCR_ORGN0_SHIFT)
#define TCR_ORGN0_WBnWA		(UL(3) << TCR_ORGN0_SHIFT)

#define TCR_ORGN1_SHIFT		26
#define TCR_ORGN1_MASK		(UL(3) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_NC		(UL(0) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WBWA		(UL(1) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WT		(UL(2) << TCR_ORGN1_SHIFT)
#define TCR_ORGN1_WBnWA		(UL(3) << TCR_ORGN1_SHIFT)

#define TCR_ORGN_NC		(TCR_ORGN0_NC | TCR_ORGN1_NC)
#define TCR_ORGN_WBWA		(TCR_ORGN0_WBWA | TCR_ORGN1_WBWA)
#define TCR_ORGN_WT		(TCR_ORGN0_WT | TCR_ORGN1_WT)
#define TCR_ORGN_WBnWA		(TCR_ORGN0_WBnWA | TCR_ORGN1_WBnWA)
#define TCR_ORGN_MASK		(TCR_ORGN0_MASK | TCR_ORGN1_MASK)

#define TCR_SH0_SHIFT		12
#define TCR_SH0_MASK		(UL(3) << TCR_SH0_SHIFT)
#define TCR_SH0_INNER		(UL(3) << TCR_SH0_SHIFT)

#define TCR_SH1_SHIFT		28
#define TCR_SH1_MASK		(UL(3) << TCR_SH1_SHIFT)
#define TCR_SH1_INNER		(UL(3) << TCR_SH1_SHIFT)
#define TCR_SHARED		(TCR_SH0_INNER | TCR_SH1_INNER)

#define TCR_TG0_SHIFT		14
#define TCR_TG0_MASK		(UL(3) << TCR_TG0_SHIFT)
#define TCR_TG0_4K		(UL(0) << TCR_TG0_SHIFT)
#define TCR_TG0_64K		(UL(1) << TCR_TG0_SHIFT)
#define TCR_TG0_16K		(UL(2) << TCR_TG0_SHIFT)

#define TCR_TG1_SHIFT		30
#define TCR_TG1_MASK		(UL(3) << TCR_TG1_SHIFT)
#define TCR_TG1_16K		(UL(1) << TCR_TG1_SHIFT)
#define TCR_TG1_4K		(UL(2) << TCR_TG1_SHIFT)
#define TCR_TG1_64K		(UL(3) << TCR_TG1_SHIFT)

#define TCR_IPS_SHIFT		32
#define TCR_IPS_MASK		(UL(7) << TCR_IPS_SHIFT)
#define TCR_A1			(UL(1) << 22)
#define TCR_ASID16		(UL(1) << 36)
#define TCR_TBI0		(UL(1) << 37)
#define TCR_TBI1		(UL(1) << 38)
#define TCR_HA			(UL(1) << 39)
#define TCR_HD			(UL(1) << 40)
#define TCR_NFD0		(UL(1) << 53)
#define TCR_NFD1		(UL(1) << 54)

/*
 * TTBR.
 */
#ifdef CONFIG_ARM64_PA_BITS_52
/*
 * This should be GENMASK_ULL(47, 2).
 * TTBR_ELx[1] is RES0 in this configuration.
 */
#define TTBR_BADDR_MASK_52	(((UL(1) << 46) - 1) << 2)
#endif

#ifdef CONFIG_ARM64_VA_BITS_52
/* Must be at least 64-byte aligned to prevent corruption of the TTBR */
#define TTBR1_BADDR_4852_OFFSET	(((UL(1) << (52 - PGDIR_SHIFT)) - \
				 (UL(1) << (48 - PGDIR_SHIFT))) * 8)
#endif

#endif
