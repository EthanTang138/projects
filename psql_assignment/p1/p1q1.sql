-- Ethan Tang, tangeth4, 1009681471
SELECT COUNT(*) as "Total Seniors"
FROM Person
WHERE 2024 - DATE_PART('year', DateOfBirth) >= 65;   -- using 2024 for "as of the end of 2023"