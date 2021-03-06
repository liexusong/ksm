#ifndef __MM_H
#define __MM_H

#ifndef PXI_SHIFT
#define PXI_SHIFT			39
#endif

#ifndef PPI_SHIFT
#define PPI_SHIFT			30
#endif

#ifndef PDI_SHIFT
#define PDI_SHIFT			21
#endif

#ifndef PTI_SHIFT
#define PTI_SHIFT			12
#endif

#ifndef PTE_SHIFT
#define PTE_SHIFT			3
#endif

#define VA_BITS				48
#define VA_MASK				((1ULL << VA_BITS) - 1)
#define VA_SHIFT			16

#ifndef PTX_MASK
#define PTX_MASK			0x1FF
#endif

#ifndef PPI_MASK
#define PPI_MASK			0x3FFFF
#endif

#ifndef PDI_MASK
#define PDI_MASK			0x7FFFFFF
#endif

#ifndef PTI_MASK
#define PTI_MASK			0xFFFFFFFFF
#endif

#ifndef PXE_BASE
#define PXE_BASE			0xfffff6fb7dbed000ull
#endif

#ifndef PPE_BASE
#define PPE_BASE			0xfffff6fb7da00000ull
#endif

#ifndef PDE_BASE
#define PDE_BASE			0xfffff6fb40000000ull
#endif

#ifndef PTE_BASE
#define PTE_BASE			0xfffff68000000000ull
#endif

#ifndef PXE_TOP
#define PXE_TOP				0xFFFFF6FB7DBEDFFFULL
#endif

#ifndef PPE_TOP
#define PPE_TOP				0xFFFFF6FB7DBFFFFFULL
#endif

#ifndef PDE_TOP
#define PDE_TOP				0xFFFFF6FB7FFFFFFFULL
#endif

#ifndef PTE_TOP
#define PTE_TOP				0xFFFFF6FFFFFFFFFFULL
#endif

 /* Regular IA32E page  */
#define PAGE_PRESENT			0x1
#define PAGE_WRITE			0x2
#define PAGE_OWNER			0x4
#define PAGE_WRITETHRU			0x8
#define PAGE_CACHEDISABLE		0x10
#define PAGE_ACCESSED			0x20
#define PAGE_DIRTY			0x40
#define PAGE_LARGE			0x80
#define PAGE_GLOBAL			0x100
#define PAGE_COPYONWRITE		0x200
#define PAGE_PROTOTYPE			0x400
#define PAGE_TRANSIT			0x800
#define PAGE_MASK			(0xFFFFFFFFFULL << PAGE_SHIFT)
#define PAGE_PA(page)			((page) & PAGE_MASK)
#define PAGE_FN(page)			(((page) >> PTI_SHIFT) & PTI_MASK)
#define PAGE_SOFT_WS_IDX_SHIFT		52
#define PAGE_SOFT_WS_IDX_MASK		0x7FF
#define PAGE_NX				0x8000000000000000
#define PAGE_LPRESENT			(PAGE_PRESENT | PAGE_LARGE)

static inline int pte_soft_ws_idx(uintptr_t *pte)
{
	return (*pte >> PAGE_SOFT_WS_IDX_SHIFT) & PAGE_SOFT_WS_IDX_MASK;
}

static inline bool pte_present(uintptr_t *pte)
{
	return *pte & PAGE_PRESENT;
}

static inline bool pte_large(uintptr_t *pte)
{
	return *pte & PAGE_LARGE;
}

static inline bool pte_trans(uintptr_t *pte)
{
	return *pte & PAGE_TRANSIT;
}

static inline bool pte_prototype(uintptr_t *pte)
{
	return *pte & PAGE_PROTOTYPE;
}

static inline bool pte_large_present(uintptr_t *pte)
{
	return (*pte & PAGE_LPRESENT) == PAGE_LPRESENT;
}

static inline bool pte_swapper(uintptr_t *pte)
{
	if (pte_present(pte))
		return false;

	return pte_trans(pte) && !pte_prototype(pte);
}

static inline uintptr_t *va_to_pxe(uintptr_t va)
{
	uintptr_t off = (va >> PXI_SHIFT) & PTX_MASK;
	return (uintptr_t *)(PXE_BASE + off * sizeof(uintptr_t));
}

static inline uintptr_t *va_to_ppe(uintptr_t va)
{
	uintptr_t off = (va >> PPI_SHIFT) & PPI_MASK;
	return (uintptr_t *)(PPE_BASE + off * sizeof(uintptr_t));
}

static inline uintptr_t *va_to_pde(uintptr_t va)
{
	uintptr_t off = (va >> PDI_SHIFT) & PDI_MASK;
	return (uintptr_t *)(PDE_BASE + off * sizeof(uintptr_t));
}

static inline uintptr_t *va_to_pte(uintptr_t va)
{
	uintptr_t off = (va >> PTI_SHIFT) & PTI_MASK;
	return (uintptr_t *)(PTE_BASE + off * sizeof(uintptr_t));
}

static inline uintptr_t __pte_to_va(uintptr_t *pte)
{
	return ((((uintptr_t)pte - PTE_BASE) << (PAGE_SHIFT + VA_SHIFT - PTE_SHIFT)) >> VA_SHIFT);
}

static inline void *pte_to_va(uintptr_t *pte)
{
	return (void *)__pte_to_va(pte);
}

static inline uintptr_t va_to_pa(uintptr_t va)
{
	uintptr_t *pte = va_to_pte(va);
	if (!pte_present(pte))
		return 0;

	return PAGE_PA(*pte) | (va & ((1ULL << PAGE_SHIFT) - 1));
}

static inline bool consult_vad(uintptr_t va)
{
	return !pte_present(va_to_pde(va)) || *va_to_pte(va) == 0;
}

static inline bool is_software_pte(uintptr_t *pte)
{
	return !pte_trans(pte) && !pte_prototype(pte);
}

static inline bool is_subsection_pte(uintptr_t *pte)
{
	return !pte_present(pte) && pte_prototype(pte);
}

static inline bool is_demandzero_pte(uintptr_t *pte)
{
	return !pte_present(pte) && !pte_prototype(pte) && !pte_trans(pte);
}

static inline bool is_phys(uintptr_t va)
{
	return pte_present(va_to_pxe(va)) && pte_present(va_to_ppe(va)) &&
		(pte_large_present(va_to_pde(va)) || pte_present(va_to_pte(va)));
}

/* Transitition page  (Unique defines only...)  */
#define PTT_PROTECTION_SHIFT	5
#define PTT_PROTECTION_MASK	0x1F

static inline u8 ptt_protection(uintptr_t *pte)
{
	return (*pte >> PTT_PROTECTION_SHIFT) & PTT_PROTECTION_MASK;
}

/* Prototype PTE  (Unique defines only...)  */
#define PRT_PROTECTION_SHIFT		11
#define PRT_PROTECTION_MASK		0x3F
#define PRT_PROTO_ADDRESS_SHIFT		VA_SHIFT
#define PRT_PROTO_ADDRESS_MASK		VA_MASK
#define PRT_READONLY			0x100

static inline u8 prt_prot(uintptr_t *pte)
{
	return (*pte >> PRT_PROTECTION_SHIFT) & PRT_PROTECTION_MASK;
}

static inline uintptr_t prt_addr(uintptr_t *pte)
{
	return (*pte >> PRT_PROTO_ADDRESS_SHIFT) & PRT_PROTO_ADDRESS_MASK;
}

static inline bool prt_ro(uintptr_t *pte)
{
	return *pte & PRT_READONLY;
}

static inline bool prt_is_vad(uintptr_t *pte)
{
	return prt_addr(pte) == 0xFFFFFFFF0000;
}

/* Software PTE  */
#define SPTE_PF_LO_SHIFT	1			/* Number of page file (up to 16) */
#define SPTE_PF_LO_MASK		0x1F
#define SPTE_PF_HI_SHIFT	32			/* Page file offset (multiple of PAGE_SIZE)  */
#define SPTE_PF_HI_MASK		0xFFFFFFFF
#define SPTE_IN_STORE_MASK	0x400000
#define SPTE_PROTECTION_SHIFT	5
#define SPTE_PROTECTION_MASK	0x1F

static inline bool spte_in_store(uintptr_t *spte)
{
	return *spte & SPTE_IN_STORE_MASK;
}

static inline bool spte_prot(uintptr_t *spte)
{
	return (*spte >> SPTE_PROTECTION_SHIFT) & SPTE_PROTECTION_MASK;
}

#endif
