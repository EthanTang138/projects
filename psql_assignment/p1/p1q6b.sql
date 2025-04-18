-- Ethan Tang, tangeth4, 1009681471
-- b)
SELECT s.RouteID, COUNT(t.SIN) as "number of times"
FROM Ship s
JOIN Take t ON t.ShipID = s.ShipID
WHERE t.date = '2023-09-07'
GROUP BY s.RouteID
ORDER BY COUNT(t.SIN) DESC
LIMIT 1;