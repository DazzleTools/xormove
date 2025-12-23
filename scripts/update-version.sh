#!/bin/bash
#
# Manual version update script for xormove
# Updates version string in version.h without requiring a commit
# Useful for testing and manual version synchronization
#
# Usage:
#   ./scripts/update-version.sh [--auto]
#
# Options:
#   --auto    Run in automatic mode (for git hooks)
#   --build   Use today's date (for creating builds/releases)
#   --commit  Use last commit date (for version fixes)
#

set -e

# Parse arguments
AUTO_MODE=false
BUILD_MODE=false
COMMIT_MODE=false
EXPLICIT_DATE=""
for arg in "$@"; do
    case $arg in
        --auto)
            AUTO_MODE=true
            ;;
        --build)
            BUILD_MODE=true
            ;;
        --commit)
            COMMIT_MODE=true
            ;;
        --date)
            shift
            EXPLICIT_DATE="$1"
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --auto       Run in automatic mode (for git hooks)"
            echo "  --build      Use today's date (for creating builds/releases)"
            echo "  --commit     Use last commit date (for version fixes)"
            echo "  --date DATE  Use explicit date (YYYYMMDD format)"
            echo ""
            echo "Default behavior:"
            echo "  - If working directory has changes: uses today's date"
            echo "  - If working directory is clean: uses last commit date"
            exit 0
            ;;
        *)
            ;;
    esac
done

# Colors for output (only in interactive mode)
if [ "$AUTO_MODE" = false ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m' # No Color

    echo -e "${BLUE}xormove Version Updater${NC}"
    echo "========================"
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

# Configuration
SOURCE_FILE="include/version.h"

# Check if we're in the right directory
if [ ! -f "$SOURCE_FILE" ]; then
    # Try from scripts directory
    if [ -f "../$SOURCE_FILE" ]; then
        cd ..
    else
        echo -e "${RED}Error:${NC} $SOURCE_FILE not found"
        echo "Please run from the project root directory"
        exit 1
    fi
fi

[ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Updating version in $SOURCE_FILE"

# Extract base version components from version.h
MAJOR=$(grep -E "^#define XORMOVE_VERSION_MAJOR [0-9]+" "$SOURCE_FILE" | awk '{print $3}')
MINOR=$(grep -E "^#define XORMOVE_VERSION_MINOR [0-9]+" "$SOURCE_FILE" | awk '{print $3}')
PATCH=$(grep -E "^#define XORMOVE_VERSION_PATCH [0-9]+" "$SOURCE_FILE" | awk '{print $3}')
PHASE=$(grep -E "^#define XORMOVE_VERSION_PHASE" "$SOURCE_FILE" | sed 's/.*"\(.*\)".*/\1/' | tr -d ' ')

# Validate extraction
if [ -z "$MAJOR" ] || [ -z "$MINOR" ] || [ -z "$PATCH" ]; then
    echo -e "${RED}Error:${NC} Could not extract version components from $SOURCE_FILE"
    echo "Found: MAJOR=$MAJOR, MINOR=$MINOR, PATCH=$PATCH"
    exit 1
fi

BASE_VERSION="$MAJOR.$MINOR.$PATCH"
# Add phase suffix if present
if [ -n "$PHASE" ]; then
    VERSION_WITH_PHASE="$BASE_VERSION-$PHASE"
else
    VERSION_WITH_PHASE="$BASE_VERSION"
fi
[ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Base version: $BASE_VERSION"

# Get git information with better handling
BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")
BUILD_COUNT=$(git rev-list --count HEAD 2>/dev/null || echo "0")

# Determine date to use
if [ -n "$EXPLICIT_DATE" ]; then
    DATE=$EXPLICIT_DATE
    [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Using explicit date: $DATE"
elif [ "$BUILD_MODE" = true ]; then
    DATE=$(date +%Y%m%d)
    [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Using today's date (build mode): $DATE"
elif [ "$COMMIT_MODE" = true ]; then
    # Use last commit date
    if git log -1 --format=%cd --date=format:%Y%m%d >/dev/null 2>&1; then
        DATE=$(git log -1 --format=%cd --date=format:%Y%m%d)
        [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Using last commit date: $DATE"
    else
        DATE=$(date +%Y%m%d)
        [ "$AUTO_MODE" = false ] && echo -e "${YELLOW}[Version]${NC} No commits found, using today's date: $DATE"
    fi
else
    # Default: Check if working tree is dirty
    if [ -n "$(git status --porcelain 2>/dev/null)" ]; then
        # Working directory has changes - use today's date
        DATE=$(date +%Y%m%d)
        [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Working directory has changes, using today's date: $DATE"
    else
        # Working directory is clean - use last commit date
        if git log -1 --format=%cd --date=format:%Y%m%d >/dev/null 2>&1; then
            DATE=$(git log -1 --format=%cd --date=format:%Y%m%d)
            [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Working directory clean, using last commit date: $DATE"
        else
            DATE=$(date +%Y%m%d)
            [ "$AUTO_MODE" = false ] && echo -e "${YELLOW}[Version]${NC} No commits found, using today's date: $DATE"
        fi
    fi
fi

# Get commit hash (short version)
if git rev-parse --short HEAD >/dev/null 2>&1; then
    COMMIT_HASH=$(git rev-parse --short HEAD 2>/dev/null)
else
    COMMIT_HASH="unknown"
fi

# Construct new version string
NEW_VERSION="${VERSION_WITH_PHASE}_${BRANCH}_${BUILD_COUNT}-${DATE}-${COMMIT_HASH}"

# Get current version from file
CURRENT_VERSION=$(grep '#define XORMOVE_VERSION_FULL' "$SOURCE_FILE" | sed 's/.*"\(.*\)".*/\1/')

# Only update if version changed
if [ "$NEW_VERSION" != "$CURRENT_VERSION" ]; then
    [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Updating: $CURRENT_VERSION → $NEW_VERSION"

    # Update the version string in the file
    sed -i "s/^#define XORMOVE_VERSION_FULL \".*\"/#define XORMOVE_VERSION_FULL \"$NEW_VERSION\"/" "$SOURCE_FILE"

    # Also update VERSION_STRING and VERSION_WITH_PHASE
    sed -i "s/^#define XORMOVE_VERSION_STRING \".*\"/#define XORMOVE_VERSION_STRING \"$BASE_VERSION\"/" "$SOURCE_FILE"
    sed -i "s/^#define XORMOVE_VERSION_WITH_PHASE \".*\"/#define XORMOVE_VERSION_WITH_PHASE \"$VERSION_WITH_PHASE\"/" "$SOURCE_FILE"

    # Stage the file for commit if in auto mode
    if [ "$AUTO_MODE" = true ]; then
        git add "$SOURCE_FILE" 2>/dev/null || true
    fi

    if [ "$AUTO_MODE" = false ]; then
        echo -e "${GREEN}✓${NC} Version updated successfully"
        echo ""
        echo -e "${BLUE}New version:${NC} $NEW_VERSION"
        echo ""
        echo "Components:"
        echo "  • Base: $BASE_VERSION"
        echo "  • Branch: $BRANCH"
        echo "  • Build: $BUILD_COUNT"
        echo "  • Date: $DATE"
        echo "  • Commit: $COMMIT_HASH"
    fi
else
    [ "$AUTO_MODE" = false ] && echo -e "${GREEN}[Version]${NC} Already up to date: $CURRENT_VERSION"
fi
