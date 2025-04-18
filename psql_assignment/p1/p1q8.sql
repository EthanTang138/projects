-- Ethan Tang, tangeth4, 1009681471
SELECT p.FirstName, p.LastName, pi.SIN
FROM Pilot pi
JOIN Person p ON pi.SIN = p.SIN
LEFT JOIN Infraction i ON pi.SIN = i.SIN
GROUP BY p.FirstName, p.LastName, pi.SIN
HAVING pi.YearsOfService > 5
   AND pi.Salary > 75000
   AND (SUM(i.Demerit) IS NULL OR SUM(i.Demerit) < 9);
