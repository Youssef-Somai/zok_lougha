-- ============================================
-- Database Rollback Script
-- Project: Summer Club Material Management
-- Purpose: Remove image storage support from MATERIEL table
-- Date: 2025-11-30
-- ============================================

-- This script removes the IMAGE_DATA column from the MATERIEL table
-- Use this ONLY if you need to rollback the migration
-- WARNING: This will delete all stored images permanently!

-- Drop the IMAGE_DATA column
ALTER TABLE MATERIEL DROP COLUMN IMAGE_DATA;

-- Verify the column was removed successfully
SELECT COLUMN_NAME, DATA_TYPE, DATA_LENGTH, NULLABLE
FROM USER_TAB_COLUMNS
WHERE TABLE_NAME = 'MATERIEL'
ORDER BY COLUMN_ID;

-- ============================================
-- Rollback Complete
-- ============================================
