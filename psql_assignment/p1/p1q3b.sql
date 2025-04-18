-- Ethan Tang, tangeth4, 1009681471
-- b)
SELECT pe.FirstName, pe.LastName, p.YearsOfService, s.ShipID
FROM Ship s
JOIN Operate o ON s.ShipID = o.ShipID
JOIN Pilot p ON o.SIN = p.SIN
JOIN Person pe on pe.SIN = p.SIN
WHERE s.AdvertisingRevenue = (
    SELECT MAX(AdvertisingRevenue)
    FROM Ship
    WHERE AdvertisingRevenue > 10000
    );