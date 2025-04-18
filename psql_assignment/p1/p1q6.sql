-- Ethan Tang, tangeth4, 1009681471
-- a)
SELECT MAX(s.ShipID) as ShipID, s.Manufacturer
FROM Ship s
GROUP BY s.Manufacturer
HAVING COUNT(s.ShipID) = 1;

-- b)
SELECT s.RouteID, COUNT(t.SIN) as "number of times"
FROM Ship s
JOIN Take t ON t.ShipID = s.ShipID
WHERE t.date = '2023-09-07'
GROUP BY s.RouteID
ORDER BY COUNT(t.SIN) DESC
LIMIT 1;

-- c)
SELECT t.Date, COUNT(t.SIN) AS "trips taken"
FROM Take t
GROUP BY t.Date
ORDER BY COUNT(t.SIN) DESC
LIMIT 1;
