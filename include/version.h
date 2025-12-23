/**
 * Version information for xormove.
 *
 * This file is automatically updated by git pre-commit hooks.
 * Format: VERSION_BRANCH_BUILD-YYYYMMDD-COMMITHASH
 *
 * Example: 0.1.0_main_3-20251223-88bd441
 *
 * Components:
 * - VERSION: Semantic version (MAJOR.MINOR.PATCH)
 * - BRANCH: Git branch name
 * - BUILD: Commit count
 * - YYYYMMDD: Commit date
 * - COMMITHASH: Short commit hash
 */

#ifndef XORMOVE_VERSION_H
#define XORMOVE_VERSION_H

// Semantic version components
#define XORMOVE_VERSION_MAJOR 0
#define XORMOVE_VERSION_MINOR 1
#define XORMOVE_VERSION_PATCH 3

// Optional release phase (alpha, beta, rc1, rc2, etc.)
// Leave empty for stable releases
#define XORMOVE_VERSION_PHASE ""

// Full version string - updated by git pre-commit hook
// DO NOT EDIT THIS LINE MANUALLY
// Note: Hash reflects the commit this version builds upon (HEAD at commit time)
#define XORMOVE_VERSION_FULL "0.1.3_dev_9-20251223-0533c69"

// Helper macros
#define XORMOVE_VERSION_STRING "0.1.3"
#define XORMOVE_VERSION_WITH_PHASE "0.1.3"

// Stringification helpers
#define XORMOVE_STRINGIFY(x) #x
#define XORMOVE_TOSTRING(x) XORMOVE_STRINGIFY(x)

// Version as single integer for comparisons: MAJOR*10000 + MINOR*100 + PATCH
#define XORMOVE_VERSION_NUMBER ((XORMOVE_VERSION_MAJOR * 10000) + (XORMOVE_VERSION_MINOR * 100) + XORMOVE_VERSION_PATCH)

#endif // XORMOVE_VERSION_H
