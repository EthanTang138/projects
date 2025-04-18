-- Ethan Tang, tangeth4, 1009681471
SELECT COUNT(*)
FROM Take t
JOIN Ship s ON t.ShipID = s.ShipID
WHERE s.RouteID = 1
  AND t.Date = '2023-09-04';
