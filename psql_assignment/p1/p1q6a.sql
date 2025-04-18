-- Ethan Tang, tangeth4, 1009681471
-- a)
SELECT MAX(s.ShipID) as ShipID, s.Manufacturer
FROM Ship s
GROUP BY s.Manufacturer
HAVING COUNT(s.ShipID) = 1;