-- Ethan Tang, tangeth4, 1009681471
-- c)
SELECT s.RouteID, SUM(s.AdvertisingRevenue) AS "Total Revenue"
FROM Ship s
GROUP BY s.RouteID
ORDER BY SUM(s.AdvertisingRevenue) DESC;