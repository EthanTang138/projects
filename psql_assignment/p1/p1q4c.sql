-- Ethan Tang, tangeth4, 1009681471
-- c)
SELECT p.Type, SUM(f.Fee) AS revenue
FROM Passenger p
JOIN Fare f ON p.Type = f.Type
JOIN Take t ON p.SIN = t.SIN
WHERE t.Date = '2023-09-01'
GROUP BY p.Type
ORDER BY SUM(f.Fee) DESC
LIMIT 1;
