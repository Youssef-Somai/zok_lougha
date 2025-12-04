-- ============================================
-- Database Migration Script
-- Project: Summer Club Material Management
-- Purpose: Add image storage support to MATERIEL table
-- Date: 2025-11-30
-- ============================================

-- This script adds the IMAGE_DATA column to store material images as BLOB
-- Execute this script on your Oracle database before running the updated application

-- Add IMAGE_DATA column to MATERIEL table
-- Using BLOB datatype for binary large object storage
ALTER TABLE MATERIEL ADD IMAGE_DATA BLOB;

-- Optional: Add a comment to document the column
COMMENT ON COLUMN MATERIEL.IMAGE_DATA IS 'Stores the material image as a binary large object (BLOB). Images are captured via camera or uploaded from device.';

-- Verify the column was added successfully
SELECT COLUMN_NAME, DATA_TYPE, DATA_LENGTH, NULLABLE
FROM USER_TAB_COLUMNS
WHERE TABLE_NAME = 'MATERIEL'
ORDER BY COLUMN_ID;

-- ============================================
-- Migration Complete
-- ============================================
-- You can now run the Qt application with image support enabled.
-- The application will automatically handle image storage and retrieval.
-- ============================================
