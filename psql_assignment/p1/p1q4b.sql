-- Ethan Tang, tangeth4, 1009681471
-- b)
SELECT p.Type, SUM(f.Fee) AS revenue
FROM Passenger p
JOIN Fare f ON p.Type = f.Type
GROUP BY p.Type
HAVING SUM(f.Fee) > 500;
