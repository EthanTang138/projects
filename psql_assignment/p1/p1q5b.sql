-- Ethan Tang, tangeth4, 1009681471
-- b)
SELECT pi.SIN, SUM(i.Demerit) AS "Total Points", 
       SUM(i.Fine) AS "Total Fine"
FROM Pilot pi
JOIN Infraction i ON pi.SIN = i.SIN
GROUP BY pi.SIN
HAVING SUM(i.Demerit) >= 2
ORDER BY SUM(i.Demerit) DESC, SUM(i.Fine) DESC;