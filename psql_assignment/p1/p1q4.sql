-- Ethan Tang, tangeth4, 1009681471
-- a)
SELECT p.Type, SUM(f.Fee) AS revenue
FROM Passenger p
JOIN Fare f ON p.Type = f.Type
GROUP BY p.Type;

-- b)
SELECT p.Type, SUM(f.Fee) AS revenue
FROM Passenger p
JOIN Fare f ON p.Type = f.Type
GROUP BY p.Type
HAVING SUM(f.Fee) > 500;

-- c)
SELECT p.Type, SUM(f.Fee) AS revenue
FROM Passenger p
JOIN Fare f ON p.Type = f.Type
JOIN Take t ON p.SIN = t.SIN
WHERE t.Date = '2023-09-01'
GROUP BY p.Type
ORDER BY SUM(f.Fee) DESC
LIMIT 1;
