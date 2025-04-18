-- Ethan Tang, tangeth4, 1009681471
SELECT p.FirstName, p.LastName, p.Sex AS Gender, ph.Number AS PhoneNumber
FROM Person p
JOIN Take t ON t.SIN = p.SIN
JOIN Ship sh ON t.ShipID = sh.ShipID
JOIN Schedule sc ON sh.RouteID = sc.RouteID
JOIN Stop st ON st.StopID = sc.StopID
JOIN Sites s ON st.SIName = s.SIName
JOIN Event e ON e.SIName = s.SIName AND e.PhoneNumber = s.PhoneNumber
LEFT JOIN Phone ph ON p.SIN = ph.SIN  -- left join since not all students have phone numbers
WHERE e.EName = 'Jedi Knight Basketball'
  AND s.SIName = 'Jedi Temple'
  AND sh.RouteID = 4;