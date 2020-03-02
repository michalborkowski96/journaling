convert :: Read a => String -> [a]
convert = map read . words


naturals :: Int -> [Int]
naturals 0 = []
naturals i = ((i - 1) : (naturals $ i - 1))

sumList [] = 0
sumList (h:t) = h + sumList t

fluctuatingSum l = let (r, evenLength) = fluctuatingSum' l in r
	where
		fluctuatingSum' [] = (0, True)
		fluctuatingSum' (h:t) = case fluctuatingSum' t of
			(r, True) -> (r + h, False)
			(r, False) -> (r - h, True)

getSumsAndExtendList l 0 = ([], l)
getSumsAndExtendList l i =
	let (prevSums, prevList) = getSumsAndExtendList l (i - 1) in
	let newList = (i - 1 : prevList) in
	((sumList newList : prevSums), newList)

performTest limit1 limit2 =
	let baseInts = naturals limit1 in
	let (sums, finalList) = getSumsAndExtendList baseInts limit2 in
	(sumList finalList, fluctuatingSum sums)

main = do
	line <- getLine
	let (limit1:limit2:_) = (convert line :: [Int])
	let (s1, s2) = performTest limit1 limit2
	putStrLn $ show s1
	putStrLn $ show s2
	return ()
