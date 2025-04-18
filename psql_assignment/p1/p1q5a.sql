-- Ethan Tang, tangeth4, 1009681471
-- a)
SELECT p.SIN, p.FirstName, p.LastName, 
       DATE_PART('year', CURRENT_DATE) - DATE_PART('year', p.DateOfBirth) AS Age
FROM Pilot pi
JOIN Person p ON pi.SIN = p.SIN
LEFT JOIN Infraction i ON pi.SIN = i.SIN
GROUP BY p.SIN, p.FirstName, p.LastName, p.DateOfBirth
HAVING COUNT(i.SIN) < 3;